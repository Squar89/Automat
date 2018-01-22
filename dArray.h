#ifndef _D_ARRAY_
#define _D_ARRAY_

#include <stdbool.h>

#define STARTSIZE 1

typedef struct dArray {
    unsigned int *array_start;
    unsigned int capacity;
    unsigned int size;
} dArray;

dArray* setup();

void expand(dArray *array);

void push(dArray *array, unsigned int newElement);

void pop_back(dArray *array);

void clear(dArray *array);

#endif