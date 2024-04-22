#include <beet_shared/memory.h>
#include <beet_shared/assert.h>

#include <cstring>
#include <cstdlib>

//===API================================================================================================================
void *mem_zalloc(const size_t size) {
    return memset(malloc(size), 0, size);
}

void *mem_malloc(const size_t size) {
    return malloc(size);
}

void mem_free(void *block) {
    ASSERT_MSG(block != nullptr, "Err: trying to invalidate nullptr");
    free(block);
    block = nullptr;
}
//======================================================================================================================