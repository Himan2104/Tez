#include <Tez/Core/Memory.hxx>
#include <cstring>
#include <mutex>

namespace Tez
{

Chunk::Chunk(size_t capacity)
    : _capacity(capacity)
    , _offset(0)
#ifdef TEZ_ENABLE_MEMORY_STATS
    , _peakUsage(0)
    , _allocationCount(0)
#endif
{ _buffer = new uint8_t[_capacity]; }

Chunk::~Chunk() { delete[] _buffer; }

void Chunk::Resize(size_t newCapacity)
{
    std::unique_lock lock(_bufferMutex);

    // Double-check condition to prevent concurrent resize overlaps from CAS loops
    if (newCapacity <= _capacity) return;

    uint8_t* newBuffer  = new uint8_t[newCapacity];
    size_t currentUsage = _offset.load(std::memory_order_relaxed);

    if (_buffer && currentUsage > 0) { std::memcpy(newBuffer, _buffer, currentUsage); }
    delete[] _buffer;

    _buffer   = newBuffer;
    _capacity = newCapacity;
}

void Chunk::Reset()
{
    _offset.store(0, std::memory_order_release);
#ifdef TEZ_ENABLE_MEMORY_STATS
    _allocationCount.store(0, std::memory_order_relaxed);
#endif
}

#ifdef TEZ_ENABLE_MEMORY_STATS
const ChunkStats& Chunk::GetStats() const
{
    std::shared_lock lock(_bufferMutex); // Ensures _capacity sync
    _stats.totalCapacity   = _capacity;
    _stats.allocatedBytes  = _offset.load(std::memory_order_relaxed);
    _stats.peakBytes       = _peakUsage.load(std::memory_order_relaxed);
    _stats.allocationCount = _allocationCount.load(std::memory_order_relaxed);
    return _stats;
}
#endif

size_t Chunk::AlignForward(size_t currentOffset, size_t alignment) const
{ return (currentOffset + (alignment - 1)) & ~(alignment - 1); }

MemoryManager& MemoryManager::Instance()
{
    static MemoryManager instance;
    return instance;
}
} // namespace Tez
