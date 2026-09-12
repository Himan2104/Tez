#pragma once

#include <Tez/Core/Types.hxx>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <new>
#include <shared_mutex>
#include <unordered_map>

namespace Tez
{

///
/// @brief Base interface struct for all memory tags.
///
struct IMemoryTag
{
    virtual ~IMemoryTag() = default;
};

///
/// @brief Concept enforcing that a type is a valid memory tag.
///
template <typename T>
concept IsMemoryTag = std::derived_from<T, IMemoryTag>;

///
/// @brief Default tag for general engine allocations.
///
struct MemTagGeneral : public IMemoryTag
{
};

///
/// @brief Type trait mapping a given type to its target memory chunk tag.
/// @tparam T The type being mapped.
///
template <typename T>
struct MemTag
{
    using Type = MemTagGeneral;
};

///
/// @brief Type trait defining the instantiation rules for a given memory tag.
/// @tparam Tag The memory tag constrained by IsMemoryTag.
///
template <IsMemoryTag Tag>
struct MemTagRules
{
    static constexpr size_t initialBlockSize = 64 * 1024 * 1024; /// 64 MB default
};

class Chunk;

///
/// @brief A relocatable handle to memory allocated within a Chunk.
/// @tparam T The type of the object stored in the memory.
///
template <typename T>
struct Handle
{
public:
    ///
    /// @brief Checks if the handle points to a valid Chunk and allocation.
    /// @return true if the chunk is valid, false otherwise.
    ///
    bool IsValid() const;

    ///
    /// @brief Resolves the handle to a mutable pointer of the underlying type.
    /// @return T* A pointer to the resolved memory, or nullptr if invalid.
    ///
    T* Get() const;

    ///
    /// @brief Resolves the handle to a constant pointer of the underlying type.
    /// @return const T* A constant pointer to the resolved memory, or nullptr if invalid.
    ///
    const T* GetConst() const;

    Chunk* chunk  = nullptr;
    size_t offset = 0;
};

#ifdef TEZ_ENABLE_MEMORY_STATS
///
/// @brief Snapshot of the memory utilization metrics for a specific chunk.
///
struct ChunkStats
{
public:
    size_t totalCapacity   = 0;
    size_t allocatedBytes  = 0;
    size_t peakBytes       = 0;
    size_t allocationCount = 0;
};
#endif

///
/// @brief Thread-safe linear bump allocator using lock-free CAS operations.
///
class Chunk
{
public:
    ///
    /// @brief Initializes the chunk with an initial byte capacity.
    /// @param capacity The initial capacity in bytes.
    ///
    explicit Chunk(size_t capacity);

    ///
    /// @brief Destroys the chunk and frees the underlying buffer.
    ///
    ~Chunk();

    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;

    ///
    /// @brief Allocates space via a lock-free Compare-And-Swap (CAS) loop.
    /// @tparam T The type of elements to allocate.
    /// @param count The number of elements to allocate space for. Defaults to 1.
    /// @return Handle<T> A handle wrapping the chunk and memory offset.
    ///
    template <typename T>
    Handle<T> AllocateHandle(size_t count = 1)
    {
        size_t size          = sizeof(T) * count;
        size_t alignment     = alignof(T);
        size_t currentOffset = _offset.load(std::memory_order_relaxed);
        size_t alignedOffset;
        size_t nextOffset;

        do
        {
            alignedOffset = AlignForward(currentOffset, alignment);
            nextOffset    = alignedOffset + size;

            if (nextOffset > _capacity)
            {
                Resize(nextOffset * 2);
                return AllocateHandle<T>(count);
            }
        } while (!_offset.compare_exchange_weak(
            currentOffset, nextOffset, std::memory_order_acquire, std::memory_order_relaxed));

#ifdef TEZ_ENABLE_MEMORY_STATS
        _allocationCount.fetch_add(1, std::memory_order_relaxed);

        size_t currentPeak = _peakUsage.load(std::memory_order_relaxed);
        while (
            nextOffset > currentPeak &&
            !_peakUsage.compare_exchange_weak(currentPeak, nextOffset, std::memory_order_relaxed))
        {
        }
#endif

        return Handle<T>{this, alignedOffset};
    }

    ///
    /// @brief Resizes the underlying buffer, moving existing data to the new block safely.
    /// @param newCapacity The new capacity in bytes.
    ///
    void Resize(size_t newCapacity);

    ///
    /// @brief Instantly invalidates all allocations by resetting the bump pointer to zero.
    ///
    void Reset();

    ///
    /// @brief Safely resolves a raw offset into a typed pointer relative to the current buffer.
    /// @tparam T The type to cast the resolved pointer to.
    /// @param handleOffset The byte offset from the start of the chunk buffer.
    /// @return T* A pointer to the resolved memory address.
    ///
    template <typename T>
    T* Resolve(size_t handleOffset) const
    {
        std::shared_lock lock(_bufferMutex);
        return reinterpret_cast<T*>(_buffer + handleOffset);
    }

#ifdef TEZ_ENABLE_MEMORY_STATS
    ///
    /// @brief Updates and retrieves a reference to the usage metrics of the chunk.
    /// @return const ChunkStats& The telemetry snapshot.
    ///
    const ChunkStats& GetStats() const;
#endif

private:
    ///
    /// @brief Calculates the next memory offset that satisfies the alignment requirement.
    ///
    size_t AlignForward(size_t currentOffset, size_t alignment) const;

    mutable std::shared_mutex _bufferMutex;
    uint8_t* _buffer = nullptr;
    size_t _capacity;
    std::atomic<size_t> _offset;

#ifdef TEZ_ENABLE_MEMORY_STATS
    mutable ChunkStats _stats;
    std::atomic<size_t> _peakUsage;
    std::atomic<size_t> _allocationCount;
#endif
};

template <typename T>
inline bool Handle<T>::IsValid() const
{ return chunk != nullptr; }

template <typename T>
inline T* Handle<T>::Get() const
{ return chunk ? chunk->Resolve<T>(offset) : nullptr; }

template <typename T>
inline const T* Handle<T>::GetConst() const
{ return chunk ? chunk->Resolve<const T>(offset) : nullptr; }

///
/// @brief Central manager that lazily creates and manages memory chunks based on tags.
///
class MemoryManager
{
public:
    static MemoryManager& Instance();

    template <IsMemoryTag Tag>
    Chunk* GetOrCreateChunk()
    {
        UInt64 typeIdx(TypeID<Tag>());

        {
            std::shared_lock lock(_rwMutex);
            if (auto it = _chunks.find(typeIdx); it != _chunks.end()) { return it->second.get(); }
        }

        std::unique_lock lock(_rwMutex);
        if (auto it = _chunks.find(typeIdx); it != _chunks.end()) { return it->second.get(); }

        size_t capacity = MemTagRules<Tag>::initialBlockSize;
        auto newChunk   = std::make_unique<Chunk>(capacity);
        Chunk* chunkPtr = newChunk.get();

        _chunks[typeIdx] = std::move(newChunk);
        return chunkPtr;
    }

    template <IsMemoryTag Tag>
    void ResetChunk()
    {
        std::shared_lock lock(_rwMutex);
        UInt64 typeIdx(TypeID<Tag>());
        if (auto it = _chunks.find(typeIdx); it != _chunks.end()) { it->second->Reset(); }
    }

private:
    MemoryManager() = default;

    mutable std::shared_mutex _rwMutex;
    std::unordered_map<UInt64, std::unique_ptr<Chunk>> _chunks;
};

///
/// @brief Allocates and constructs a single object, routed to a Chunk via MemTag<T>.
///
template <typename T, typename... Args>
Handle<T> New(Args&&... args)
{
    using TargetTag = typename MemTag<T>::Type;
    Chunk* chunk    = MemoryManager::Instance().GetOrCreateChunk<TargetTag>();
    if (!chunk) return Handle<T>{nullptr, 0};

    Handle<T> handle = chunk->AllocateHandle<T>(1);
    T* memory        = handle.Get();
    if (memory) { new (memory) T(std::forward<Args>(args)...); }
    return handle;
}

///
/// @brief Allocates a contiguous, uninitialized memory block for an array, routed via MemTag<T>.
/// @tparam T The type of elements.
/// @param count The number of elements to allocate space for.
/// @return Handle<T> A handle to the start of the uninitialized memory.
///
template <typename T>
Handle<T> NewArray(size_t count)
{
    using TargetTag = typename MemTag<T>::Type;
    Chunk* chunk    = MemoryManager::Instance().GetOrCreateChunk<TargetTag>();
    if (!chunk) return Handle<T>{nullptr, 0};

    // Return the handle wrapping raw, uninitialized memory
    return chunk->AllocateHandle<T>(count);
}

///
/// @brief Calls destructors for the given object(s). Memory is abandoned until Chunk resets.
///
template <typename T>
void Delete(Handle<T> handle, size_t count = 1)
{
    if (!handle.IsValid()) return;

    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        T* ptr = handle.Get();
        if (ptr)
        {
            for (size_t i = 0; i < count; ++i) { ptr[i].~T(); }
        }
    }
    // Note: We do NOT free the memory block. Bump allocators reclaim strictly on Reset().
}

///
/// @brief STL-compatible allocator routing memory to the engine's tagged Chunks.
/// @warning Standard STL containers use raw pointers. If the Chunk resizes, pointers dangle.
///
template <typename T>
class StlAllocator
{
public:
    using value_type = T;

    StlAllocator() = default;
    template <class U>
    constexpr StlAllocator(const StlAllocator<U>&) noexcept
    {
    }

    T* allocate(std::size_t n)
    {
        using TargetTag = typename MemTag<T>::Type;
        Chunk* chunk    = MemoryManager::Instance().GetOrCreateChunk<TargetTag>();
        if (!chunk) throw std::bad_alloc();

        Handle<T> handle = chunk->AllocateHandle<T>(n);
        return handle.Get();
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        // No-op for bump allocators. The STL container will call destructors,
        // but the memory is abandoned until the chunk Resets.
    }
};

template <class T, class U>
bool operator==(const StlAllocator<T>&, const StlAllocator<U>&)
{ return true; }
template <class T, class U>
bool operator!=(const StlAllocator<T>&, const StlAllocator<U>&)
{ return false; }

} // namespace Tez
