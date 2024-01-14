#ifndef BEETROOT_MEMORY_H
#define BEETROOT_MEMORY_H

void *mem_zalloc(size_t size);
void *mem_malloc(size_t size);

void mem_free(void *block);

#endif //BEETROOT_MEMORY_H
