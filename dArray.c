#include <stdlib.h>
#include "dArray.h"

dArray* setup() {
    dArray *array;
    array = (dArray*) malloc(sizeof(dArray));

    unsigned int *array_start;
    array_start = (unsigned int*) malloc(STARTSIZE * sizeof(unsigned int));

    array->array_start = array_start;
    array->capacity = STARTSIZE;
    array->size = 0;

    return array;
}

void expand(dArray *array) {
    unsigned int oldCapacity = array->capacity;

    array->array_start = (unsigned int*) realloc(array->array_start, oldCapacity * 2 * sizeof(unsigned int));
    array->capacity *= 2;
}

void push(dArray *array, unsigned int newElement) {
    if (array->size == array->capacity) {
        expand(array);
    }

    *(array->array_start + array->size) = newElement;
    array->size++;
}

void pop_back(dArray *array) {
    if (array->size > 0) {
        array->size--;
    }
}

void clear(dArray *array) {
    free(array->array_start);
    free(array);
}