#include <string.h>
#include "cvec.h"

#define VECTOR_SIZE (vector->size)
#define VECTOR_CAPACITY (vector->capacity)
#define VECTOR_IS_EMPTY (!VECTOR_SIZE)
#define VECTOR_ARRAY (vector->array)
#define LAST_INDEX (VECTOR_SIZE-1)
#define CAPACITY_DEFAULT 10

CVec *cvec_create_cap(size_t capacity) {
    if (capacity == 0) return NULL;

    CVec *vector = (CVec *) malloc(sizeof(CVec));
    if (!vector) return NULL;

    VECTOR_ARRAY = (char *) malloc(sizeof(char) * capacity);
    if (!VECTOR_ARRAY) {
        free(vector);
        return NULL;
    }

    VECTOR_SIZE = 0;
    VECTOR_CAPACITY = capacity;

    return vector;
}

CVec *cvec_create() {
    return cvec_create_cap(CAPACITY_DEFAULT);
}

void cvec_expand(CVec* vector) {
    if (!vector) return;

    char *temp_array = VECTOR_ARRAY;
    
    VECTOR_ARRAY = (char *) malloc(sizeof(char) * VECTOR_CAPACITY * 2);
    if (!VECTOR_ARRAY) return;

    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(char));

    VECTOR_CAPACITY *= 2;
    free(temp_array);
}

void cvec_append(CVec* vector, char c) {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) cvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE++] = c;
}

void cvec_append_array(CVec *vector, const char *string, size_t len) {
    if (!vector || !string) return;

    while (VECTOR_SIZE + len > VECTOR_CAPACITY) {
        cvec_expand(vector);
    }
    
    memcpy(&VECTOR_ARRAY[VECTOR_SIZE], string, len);
    VECTOR_SIZE += len;
}

void cvec_append_array_line(CVec *vector, const char *string, size_t len) {
    cvec_append_array(vector, string, len);
    cvec_append(vector, '\n');
}

char cvec_pop_last(CVec *vector) {
    if (!vector) return '\0';
    if VECTOR_IS_EMPTY return '\0';

    char c = VECTOR_ARRAY[LAST_INDEX];
    --VECTOR_SIZE;

    return c;
}

size_t cvec_size(CVec* vector) {
    if (!vector) return 0;

    return VECTOR_SIZE;
}

int cvec_is_empty(CVec* vector) {
    if (!vector) return 0;

    return VECTOR_IS_EMPTY;
}

void cvec_clear(CVec *vector) {
    if (!vector) return;

    VECTOR_SIZE = 0;
}

void cvec_print(CVec *vector) {
    if (!vector) return;

    cvec_append(vector, '\0');
    fputs(vector->array, stdout);
    cvec_pop_last(vector);
}

void cvec_printf(CVec *vector, FILE* file) {
    if (!file || !vector) return;

    cvec_append(vector, '\0');
    fputs(vector->array, file);
    cvec_pop_last(vector);
}

void cvec_destroy(CVec *vector) {
    if (!vector) return;
    free(VECTOR_ARRAY);
    free(vector);
}
