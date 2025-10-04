#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define ALIGNMENT 16
#define ALIGN(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

void *calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0)
        return NULL;

    size_t total = nmemb * size;
    void *raw = sbrk(total + ALIGNMENT);
    if (raw == (void *)-1)
        return NULL;

    void *memory = (void *)ALIGN((uintptr_t)raw);
    memset(memory, 0, total);
    return memory;
}

void free(void *ptr) {
    return; // does nothing yet
}
