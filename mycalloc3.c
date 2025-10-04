#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define ALIGNMENT 16
#define ALIGN(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define ALIGN_DOWN(x) ((x) & ~(ALIGNMENT - 1))

typedef struct chunk {
    size_t size;
    struct chunk *prev, *next;
} chunk;

static chunk *flist = NULL;

#define HDR ALIGN(sizeof(chunk))
#define MIN_PAYLOAD 16
#define MIN_SPLIT (HDR + MIN_PAYLOAD)
#define ARENA_CHUNK (64 * 1024)

static inline void *user_from_chunk(chunk *c){ return (void *)((unsigned char *)c + HDR); }
static inline chunk *chunk_from_user(void *p){ return (chunk *)((unsigned char *)p - HDR); }
static inline unsigned char *chunk_end(chunk *c){ return (unsigned char *)c + HDR + c->size; }

static void remove_free(chunk *c){
    if (c->prev) c->prev->next = c->next; else flist = c->next;
    if (c->next) c->next->prev = c->prev;
    c->prev = c->next = NULL;
}

static void insert_sorted(chunk *c){
    c->prev = c->next = NULL;
    if (!flist){ flist = c; return; }
    chunk *p = flist, *last = NULL;
    while (p && p < c){ last = p; p = p->next; }
    c->next = p; c->prev = last;
    if (p) p->prev = c;
    if (last) last->next = c; else flist = c;
}

static chunk *coalesce(chunk *c){
    if (c->prev && chunk_end(c->prev) == (unsigned char *)c){
        chunk *p = c->prev;
        p->size += HDR + c->size;
        remove_free(c);
        c = p;
    }
    if (c->next && chunk_end(c) == (unsigned char *)c->next){
        chunk *n = c->next;
        c->size += HDR + n->size;
        remove_free(n);
    }
    return c;
}

static chunk *find_fit(size_t need){
    for (chunk *x = flist; x; x = x->next)
        if (x->size >= need) return x;
    return NULL;
}

static chunk *grow_heap(size_t need){
    size_t want = need + HDR;
    if (want < ARENA_CHUNK) want = ARENA_CHUNK;
    void *raw = sbrk(want + ALIGNMENT);
    if (raw == (void *)-1) return NULL;
    uintptr_t base = (uintptr_t)raw;
    uintptr_t hdr_addr = ALIGN(base);
    size_t usable = (size_t)((unsigned char *)raw + want + ALIGNMENT - (unsigned char *)hdr_addr);
    if (usable < HDR + MIN_PAYLOAD) return NULL;
    chunk *c = (chunk *)hdr_addr;
    c->size = ALIGN_DOWN(usable - HDR);
    c->prev = c->next = NULL;
    insert_sorted(c);
    return coalesce(c);
}

void free(void *ptr){
    if (!ptr) return;
    chunk *c = chunk_from_user(ptr);
    insert_sorted(c);
    coalesce(c);
}

void *calloc(size_t nmemb, size_t size){
    if (nmemb == 0 || size == 0) return NULL;
    if (size && nmemb > ((size_t)-1) / size) return NULL;
    size_t need = ALIGN(nmemb * size);
    chunk *c = find_fit(need);
    if (!c){
        if (!(c = grow_heap(need))) return NULL;
        c = find_fit(need);
        if (!c) return NULL;
    }
    if (c->size >= need + MIN_SPLIT){
        size_t rem_payload = c->size - need;
        c->size = need;
        chunk *rem = (chunk *)((unsigned char *)c + HDR + need);
        rem->size = rem_payload - HDR;
        rem->prev = rem->next = NULL;
        chunk *prev = c->prev, *next = c->next;
        if (prev) prev->next = rem; else flist = rem;
        if (next) next->prev = rem;
        rem->prev = prev; rem->next = next;
        coalesce(rem);
    } else {
        remove_free(c);
    }
    void *user = user_from_chunk(c);
    memset(user, 0, nmemb * size);
    return user;
}
