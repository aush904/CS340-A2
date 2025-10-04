#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define ALIGNMENT 16
#define ALIGN(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

typedef struct chunk {
    size_t size;
    struct chunk *next;
} chunk;

static chunk *flist = NULL;

static void *user_from_chunk(chunk *c) {
    return (void *)((unsigned char *)c + ALIGN(sizeof(chunk)));
}

static chunk *chunk_from_user(void *p) {
    return (chunk *)((unsigned char *)p - ALIGN(sizeof(chunk)));
}

void free(void *ptr) {
    if (!ptr) return;
    chunk *c = chunk_from_user(ptr);
    c->next = flist;
    flist = c;
}

void *calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return NULL;
    size_t req = nmemb * size;

    chunk *prev = NULL, *cur = flist;
    while (cur) {
        if (cur->size >= req) {
            if (prev) prev->next = cur->next; else flist = cur->next;
            void *user = user_from_chunk(cur);
            memset(user, 0, req);
            return user;
        }
        prev = cur;
        cur = cur->next;
    }

    size_t total = ALIGN(sizeof(chunk)) + ALIGN(req) + ALIGNMENT;
    void *raw = sbrk(total);
    if (raw == (void *)-1) return NULL;

    uintptr_t base = (uintptr_t)raw;
    uintptr_t hdr_addr = ALIGN(base);
    chunk *c = (chunk *)hdr_addr;
    c->size = ALIGN(req);
    c->next = NULL;

    void *user = user_from_chunk(c);
    memset(user, 0, req);
    return user;
}
