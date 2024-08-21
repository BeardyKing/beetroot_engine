#ifndef BEETROOT_MEMORY_H
#define BEETROOT_MEMORY_H

#include <cstddef>
#include <beet_shared/feature_defines.h>

//===API================================================================================================================
void *mem_zalloc(size_t size);
void *mem_malloc(size_t size);

void mem_free(void *block);

#if CHECK_FEATURE(FEATURE_MEMORY_TRACKING)
void mem_dump_memory_info();
void mem_validate_empty();
#endif //CHECK_FEATURE(FEATURE_MEMORY_TRACKING)
//======================================================================================================================

#endif //BEETROOT_MEMORY_H
