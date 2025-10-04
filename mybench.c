#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "rand.h"

#ifndef BUFFER
#define BUFFER 100
#endif

#ifndef LOOP
#define LOOP 100000
#endif

int main() {
    srand((unsigned) time(NULL));
    void *init = sbrk(0);
    void *slots[BUFFER];
    for (int i = 0; i < BUFFER; i++) slots[i] = NULL;
    for (int i = 0; i < LOOP; i++) {
        int idx = rand() % BUFFER;
        if (slots[idx]) {
            free(slots[idx]);
            slots[idx] = NULL;
        } else {
            int size = request();
            void *p = calloc(1, size);
            if (!p) { fprintf(stderr, "calloc failed at i=%d\n", i); break; }
            *((int*)p) = 123;
            slots[idx] = p;
        }
    }
    for (int i = 0; i < BUFFER; i++) if (slots[i]) free(slots[i]);
    void *current = sbrk(0);
    long kb = (long)((current - init) / 1024);
    printf("Initial top of heap: %p\n", init);
    printf("Current top of heap: %p\n", current);
    printf("Increased by %ld KB\n", kb);
    return 0;
}
