#include <stdlib.h>
#include <math.h>
#define MAX 4000
#define MIN 8

int request() {
    double k = log(((double) MAX) / MIN);
    double r = ((double)(rand() % (int)(k * 10000))) / 10000.0;
    int size = (int)((double)MAX / exp(r));
    return size;
}
