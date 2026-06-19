#include <Tez/Core/Memory.hxx>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

namespace Tez
{

void* VirtualMemory::ReserveAndCommit(size_t size)
{
#ifdef _WIN32
    return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return ptr == MAP_FAILED ? nullptr : ptr;
#endif
}

void VirtualMemory::Free(void* ptr, size_t size)
{
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

std::unordered_map<uint64_t, IPool*> MemoryManager::_globalPools;
std::mutex MemoryManager::_registryMutex;

Byte* MemoryManager::AllocateRawPage(size_t sizeInBytes)
{ return static_cast<Byte*>(VirtualMemory::ReserveAndCommit(sizeInBytes)); }

void MemoryManager::FreeRawPage(Byte* ptr, size_t sizeInBytes)
{ VirtualMemory::Free(ptr, sizeInBytes); }

} // namespace Tez
