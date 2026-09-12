#pragma once

#include "Assert.hxx"
#include "Optional.hxx"
#include "Types.hxx"
#include <Tez/Core/Memory.hxx>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace Tez
{

template <typename T, UInt64 size>
class Array
{
public:
    constexpr void Fill(const T& value)
    {
        for (UInt64 i = 0; i < size; i++) data[i] = value;
    }

    // Templated to avoid std::function overhead
    template <typename FillPred>
    constexpr void Fill(FillPred pred)
    {
        for (UInt64 i = 0; i < size; i++) data[i] = pred(i);
    }

    [[nodiscard]] constexpr T& operator[](UInt64 pos) noexcept
    {
        TEZ_SOFT_ASSERT(pos < size, "Index Out of Bounds!", data[0]);
        return data[pos];
    }

    [[nodiscard]] constexpr const T& operator[](UInt64 pos) const noexcept
    {
        TEZ_SOFT_ASSERT(pos < size, "Index Out of Bounds!", data[0]);
        return data[pos];
    }

    [[nodiscard]] constexpr T* GetData() noexcept { return data; }
    [[nodiscard]] constexpr const T* GetData() const noexcept { return data; }

    [[nodiscard]] constexpr T* begin() noexcept { return data; }
    [[nodiscard]] constexpr const T* begin() const noexcept { return data; }
    [[nodiscard]] constexpr T* end() noexcept { return data + size; }
    [[nodiscard]] constexpr const T* end() const noexcept { return data + size; }

private:
    T data[size];
};

template <typename T>
class DynamicArray
{
public:
    using value_type     = T;
    using SizeType       = UInt64;
    using iterator       = T*;
    using const_iterator = const T*;

    constexpr DynamicArray() = default;

    constexpr DynamicArray(std::initializer_list<T> init)
    {
        Reserve(init.size());
        T* pData = _handle.Get();
        for (const auto& val : init) { new (&pData[_size++]) T(val); }
    }

    constexpr explicit DynamicArray(SizeType count)
    {
        Reserve(count);
        T* pData = _handle.Get();
        for (SizeType i = 0; i < count; ++i) { new (&pData[i]) T(); }
        _size = count;
    }

    constexpr DynamicArray(SizeType count, const T& value)
    {
        Reserve(count);
        T* pData = _handle.Get();
        for (SizeType i = 0; i < count; ++i) { new (&pData[i]) T(value); }
        _size = count;
    }

    ~DynamicArray() { Clear(); }

    void PushBack(const T& value)
    {
        if (_size == _capacity) Reserve(_capacity == 0 ? 8 : _capacity * 2);
        new (&_handle.Get()[_size]) T(value);
        _size++;
    }

    void PushBack(T&& value)
    {
        if (_size == _capacity) Reserve(_capacity == 0 ? 8 : _capacity * 2);
        new (&_handle.Get()[_size]) T(std::move(value));
        _size++;
    }

    template <typename... Args>
    T& EmplaceBack(Args&&... args)
    {
        if (_size == _capacity) Reserve(_capacity == 0 ? 8 : _capacity * 2);
        T* pData = _handle.Get();
        new (&pData[_size]) T(std::forward<Args>(args)...);
        return pData[_size++];
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args)
    {
        T* pData       = _handle.Get();
        SizeType index = pos - pData;
        TEZ_SOFT_ASSERT(index <= _size, "Emplace Out of Bounds!", pData);

        if (_size == _capacity)
        {
            Reserve(_capacity == 0 ? 8 : _capacity * 2);
            pData = _handle.Get();
        }

        for (SizeType i = _size; i > index; --i)
        {
            new (&pData[i]) T(std::move(pData[i - 1]));
            pData[i - 1].~T();
        }

        new (&pData[index]) T(std::forward<Args>(args)...);
        _size++;
        return pData + index;
    }

    [[nodiscard]]
    Optional<T> PopBack() noexcept
    {
        if (IsEmpty()) return std::nullopt;

        T* pData = _handle.Get();
        _size--;
        T value = std::move(pData[_size]);
        pData[_size].~T();

        if constexpr (std::is_pointer_v<T>)
            return value ? std::move(value) : std::nullopt;
        else
            return std::move(value);
    }

    constexpr void Clear() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            T* pData = _handle.Get();
            if (pData)
            {
                for (SizeType i = 0; i < _size; ++i) { pData[i].~T(); }
            }
        }
        _size = 0;
    }

    [[nodiscard]] constexpr SizeType Size() const noexcept { return _size; }
    [[nodiscard]] constexpr bool IsEmpty() const noexcept { return _size == 0; }

    [[nodiscard]] constexpr T& operator[](SizeType index) noexcept
    {
        T* pData = _handle.Get();
        TEZ_SOFT_ASSERT(index < _size, "Index Out of Bounds!", pData[0]);
        return pData[index];
    }

    constexpr const T& operator[](SizeType index) const noexcept
    {
        const T* pData = _handle.GetConst();
        TEZ_SOFT_ASSERT(index < _size, "Index Out of Bounds!", pData[0]);
        return pData[index];
    }

    [[nodiscard]] constexpr T& Front() noexcept { return _handle.Get()[0]; }
    [[nodiscard]] constexpr const T& Front() const noexcept { return _handle.GetConst()[0]; }

    [[nodiscard]] constexpr T& Back() noexcept { return _handle.Get()[_size - 1]; }
    [[nodiscard]] constexpr const T& Back() const noexcept { return _handle.GetConst()[_size - 1]; }

    [[nodiscard]] constexpr T* Data() noexcept { return _handle.Get(); }
    [[nodiscard]] constexpr const T* Data() const noexcept { return _handle.GetConst(); }

    [[nodiscard]] constexpr iterator Begin() noexcept { return _handle.Get(); }
    [[nodiscard]] constexpr const_iterator Begin() const noexcept { return _handle.GetConst(); }

    [[nodiscard]] constexpr iterator End() noexcept { return _handle.Get() + _size; }
    [[nodiscard]] constexpr const_iterator End() const noexcept
    { return _handle.GetConst() + _size; }

    [[nodiscard]] constexpr iterator begin() noexcept { return Begin(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return Begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return End(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return End(); }

    constexpr void Reserve(SizeType newCapacity)
    {
        if (newCapacity <= _capacity) return;

        Handle<T> newHandle = NewArray<T>(newCapacity);
        T* newData          = newHandle.Get();

        if (_handle.IsValid())
        {
            T* oldData = _handle.Get();
            for (SizeType i = 0; i < _size; ++i)
            {
                new (&newData[i]) T(std::move(oldData[i]));
                oldData[i].~T();
            }
        }

        _handle   = newHandle;
        _capacity = newCapacity;
    }

private:
    Handle<T> _handle{};
    SizeType _size{0};
    SizeType _capacity{0};
};

} // namespace Tez
