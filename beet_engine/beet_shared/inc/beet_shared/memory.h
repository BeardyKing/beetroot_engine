#ifndef BEETROOT_MEMORY_H
#define BEETROOT_MEMORY_H

#include <cstddef>

#define BEET_MEMORY_DEBUG BEET_DEBUG

//===API================================================================================================================
void *mem_zalloc(size_t size);
void *mem_malloc(size_t size);

void mem_free(void *block);

#if BEET_MEMORY_DEBUG
void mem_dump_memory_info();
void mem_validate_empty();
#endif //BEET_MEMORY_DEBUG
//======================================================================================================================

#endif //BEETROOT_MEMORY_H
