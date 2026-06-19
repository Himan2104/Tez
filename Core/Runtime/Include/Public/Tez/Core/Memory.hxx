#pragma once

#include "Tez/Core/Span.hxx"
#include <Tez/Core/Types.hxx>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace Tez
{

class VirtualMemory
{
public:
    static void* ReserveAndCommit(SizeT size);
    static void Free(void* ptr, SizeT size);
};

template <typename T>
struct PoolConfig
{
    static constexpr SizeT InitialCapacity = 4;
};

class MemoryManager;

template <typename T>
struct Handle
{
    UInt32 index;
    UInt32 generation;

    bool IsValid() const { return index != Limits<UInt32>::max; }

    T* Get() const;
    T* operator->() const { return Get(); }
};

template <typename T>
const Handle<T> InvalidHandle = {Limits<UInt32>::max, 0};

class IPool
{
public:
    virtual ~IPool()                        = default;
    virtual SizeT GetActiveCount() const    = 0;
    virtual SizeT GetCapacity() const       = 0;
    virtual const char* GetTypeName() const = 0;
};

template <typename T>
class TypedPool : public IPool
{
private:
    T* _data;
    UInt32* _generations;
    UInt32* _freeIndices;

    SizeT _capacity;
    SizeT _activeCount;
    SizeT _freeListHead;

    std::mutex _poolMutex;

    void ResizeInternal(SizeT newCapacity)
    {
        if (newCapacity <= _capacity) return;

        SizeT newTotalBytes = newCapacity * (sizeof(T) + sizeof(UInt32) * 2);
        Byte* newRaw        = static_cast<Byte*>(VirtualMemory::ReserveAndCommit(newTotalBytes));

        T* newData             = reinterpret_cast<T*>(newRaw);
        UInt32* newGenerations = reinterpret_cast<UInt32*>(newRaw + (newCapacity * sizeof(T)));
        UInt32* newFreeIndices = reinterpret_cast<UInt32*>(newRaw + (newCapacity * sizeof(T)) +
                                                           (newCapacity * sizeof(UInt32)));

        for (SizeT i = 0; i < newCapacity; ++i) { newGenerations[i] = 1; }

        for (SizeT i = 0; i < _activeCount; ++i)
        {
            newGenerations[i] = _generations[i];
            if (_generations[i] % 2 != 0)
            {
                new (&newData[i]) T(std::move(_data[i]));
                _data[i].~T();
            }
        }

        for (SizeT i = 0; i < _freeListHead; ++i) { newFreeIndices[i] = _freeIndices[i]; }

        SizeT oldTotalBytes = _capacity * (sizeof(T) + sizeof(UInt32) * 2);
        VirtualMemory::Free(_data, oldTotalBytes);

        _data        = newData;
        _generations = newGenerations;
        _freeIndices = newFreeIndices;
        _capacity    = newCapacity;
    }

    bool IsValidInternal(Handle<T> handle) const
    {
        if (handle.index >= _activeCount && _freeListHead == 0) return false;
        return _generations[handle.index] == handle.generation;
    }

public:
    TypedPool()
    {
        _capacity     = 0;
        _activeCount  = 0;
        _freeListHead = 0;
        _data         = nullptr;
        _generations  = nullptr;
        _freeIndices  = nullptr;

        ResizeInternal(PoolConfig<T>::InitialCapacity);
    }

    ~TypedPool() override
    {
        for (SizeT i = 0; i < _activeCount; ++i)
        {
            if (_generations[i] % 2 != 0) { _data[i].~T(); }
        }
        SizeT totalBytes = _capacity * (sizeof(T) + sizeof(UInt32) * 2);
        VirtualMemory::Free(_data, totalBytes);
    }

    void Resize(SizeT newCapacity)
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        ResizeInternal(newCapacity);
    }

    template <typename... Args>
    Handle<T> Allocate(Args&&... args)
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        UInt32 index;

        if (_freeListHead > 0)
        {
            index = _freeIndices[--_freeListHead];
            _generations[index]++;
        }
        else
        {
            if (_activeCount >= _capacity) { ResizeInternal(_capacity * 2); }
            index = _activeCount++;
        }

        new (&_data[index]) T(std::forward<Args>(args)...);
        return {index, _generations[index]};
    }

    void Free(Handle<T> handle)
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        if (!IsValidInternal(handle)) return;

        _data[handle.index].~T();
        _generations[handle.index]++;
        _freeIndices[_freeListHead++] = handle.index;
    }

    T* Get(Handle<T> handle)
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        if (!IsValidInternal(handle)) return nullptr;
        return &_data[handle.index];
    }

    SizeT GetActiveCount() const override { return _activeCount - _freeListHead; }
    SizeT GetCapacity() const override { return _capacity; }
    const char* GetTypeName() const override { return NameOf<T>(); }
};

class MemoryManager
{
private:
    static std::unordered_map<uint64_t, IPool*> _globalPools;
    static std::mutex _registryMutex;

public:
    template <typename T, typename... Args>
    static Handle<T> Allocate(Args&&... args)
    { return GetPoolForType<T>().Allocate(std::forward<Args>(args)...); }

    template <typename T>
    static void Free(Handle<T> handle)
    { GetPoolForType<T>().Free(handle); }

    template <typename T>
    static T* Get(Handle<T> handle)
    { return GetPoolForType<T>().Get(handle); }

    template <typename T>
    static void Preallocate(SizeT capacity)
    { GetPoolForType<T>().Resize(capacity); }

    static Byte* AllocateRawPage(SizeT sizeInBytes);
    static void FreeRawPage(Byte* ptr, SizeT sizeInBytes);

private:
    template <typename T>
    static TypedPool<T>& GetPoolForType()
    {
        static TypedPool<T> instance;
        static bool registered = false;
        if (!registered)
        {
            std::lock_guard<std::mutex> lock(_registryMutex);
            _globalPools[TypeID<T>()] = &instance;
            registered                = true;
        }
        return instance;
    }

    template <typename T>
    friend Span<T> FindObjectsOfType();
};

template <typename T>
inline T* Handle<T>::Get() const
{ return MemoryManager::Get(*this); }

} // namespace Tez
