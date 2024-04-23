#include <beet_shared/memory.h>
#include <beet_shared/assert.h>

#include <cstring>
#include <cstdlib>

#if BEET_MEMORY_DEBUG

#include <cstdint>
#include <beet_shared/log.h>

#endif //BEET_MEMORY_DEBUG

//===INTERNAL_STRUCTS===================================================================================================
#if BEET_MEMORY_DEBUG
#define MAX_ACTIVE_DYNAMIC_ALLOCATIONS 128

struct MemoryInfo {
    void *ptrLocation;
    size_t allocSize;
};

static struct MemView {
    MemoryInfo info[MAX_ACTIVE_DYNAMIC_ALLOCATIONS] = {};
    uint32_t infoCount = 0;

    uint32_t totalAllocations = {};
    uint32_t totalFrees = {};
} s_memView;
#endif //BEET_MEMORY_DEBUG
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
#if BEET_MEMORY_DEBUG
inline static void mem_track_allocation(const MemoryInfo info) {
    if (info.ptrLocation) {
        ASSERT(s_memView.infoCount < MAX_ACTIVE_DYNAMIC_ALLOCATIONS);
        s_memView.info[s_memView.infoCount] = info;
        s_memView.infoCount++;
        s_memView.totalAllocations++;
    }
}

inline static void mem_track_free(const void *ptrLocation) {
    bool foundBlock = false;
    for (int i = 0; i < s_memView.infoCount; ++i) {
        MemoryInfo &info = s_memView.info[i];
        if (info.ptrLocation == ptrLocation) {
            info = s_memView.info[s_memView.infoCount - 1];
            s_memView.info[s_memView.infoCount - 1] = {};
            s_memView.infoCount--;
            foundBlock = true;
        }
    }
    ASSERT_MSG(foundBlock, "Err: Provided memory block was not found, potential leak or freeing allocation not from memory lib\n");
    s_memView.totalFrees++;
}
#endif //BEET_MEMORY_DEBUG
//======================================================================================================================

//===API================================================================================================================
void *mem_zalloc(const size_t size) {
    void *out = memset(malloc(size), 0, size);
#if BEET_MEMORY_DEBUG
    mem_track_allocation({.ptrLocation = out, .allocSize = size});
#endif //BEET_MEMORY_DEBUG
    return out;
}

void *mem_malloc(const size_t size) {
    void *out = malloc(size);
#if BEET_MEMORY_DEBUG
    mem_track_allocation({.ptrLocation = out, .allocSize = size});
#endif //BEET_MEMORY_DEBUG
    return out;
}

void mem_free(void *block) {
    ASSERT_MSG(block != nullptr, "Err: trying to invalidate nullptr");
#if BEET_MEMORY_DEBUG
    mem_track_free(block);
#endif //BEET_MEMORY_DEBUG
    free(block);
    block = nullptr;
}

#if BEET_MEMORY_DEBUG
void mem_dump_memory_info() {
    size_t inUseMemory = {};
    log_info(MSG_MEMORY, "===MEM_INFO_DUMP========\n");
    for (int i = 0; i < s_memView.infoCount; ++i) {
        const void *location = s_memView.info[i].ptrLocation;
        const size_t allocSize = s_memView.info[i].allocSize;
        log_info(MSG_MEMORY, "[%p], [%zu] Bytes\n", location, allocSize);
        inUseMemory += allocSize;
    }
    log_info(MSG_MEMORY, "In use Memory: [%zu] Bytes\n", inUseMemory);
    log_info(MSG_MEMORY, "Total allocs : [%u]\n", s_memView.totalAllocations);
    log_info(MSG_MEMORY, "Total frees  : [%u]\n", s_memView.totalFrees);
    log_info(MSG_MEMORY, "========================\n");
}

void mem_validate_empty() {
    size_t inUseMemory = {};
    for (int i = 0; i < s_memView.infoCount; ++i) {
        inUseMemory += s_memView.info[i].allocSize;
    }
    ASSERT(inUseMemory == 0);
    ASSERT(s_memView.totalAllocations == s_memView.totalFrees);
    ASSERT(s_memView.infoCount == 0);
}
#endif //BEET_MEMORY_DEBUG
//======================================================================================================================