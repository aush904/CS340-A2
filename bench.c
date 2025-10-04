#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define ROUNDS 10
#define LOOP 100000

int main() {
    srand((unsigned) time(NULL));
    void *init = sbrk(0);
    printf("Initial top of heap: %p\n", init);

    for (int j = 0; j < ROUNDS; j++) {
        for (int i = 0; i < LOOP; i++) {
            size_t size = (rand() % 4000) + sizeof(int);
            unsigned char *memory = calloc(1, size);
            if (memory == NULL) {
                fprintf(stderr, "calloc failed\n");
                return 1;
            }

            size_t check_limit = (size < 10) ? size : 10;
            for (size_t k = 0; k < check_limit; k++) {
                if (memory[k] != 0) {
                    fprintf(stderr, "Non-zero byte found at %zu!\n", k);
                    free(memory);
                    return 1;
                }
            }

            *((int *) memory) = 123;
            free(memory);
        }
    }

    void *current = sbrk(0);
    int allocated = (int)((current - init) / 1024);
    printf("Current top of heap: %p\n", current);
    printf("Increased by %d KB\n", allocated);
    return 0;
}
