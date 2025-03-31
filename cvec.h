#ifndef CVEC_H
#define CVEC_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *array;
    size_t size;
    size_t capacity;
} CVec;

CVec *cvec_create_cap(size_t);
CVec *cvec_create();
void cvec_expand(CVec*);
void cvec_shrink(CVec*);
void cvec_append(CVec*, char);
void cvec_append_array(CVec*, const char*, size_t);
void cvec_append_array_line(CVec*, const char*, size_t);
size_t cvec_size(CVec*);
void cvec_clear(CVec*);
void cvec_print(CVec*);
void cvec_printf(CVec*, FILE*);
void cvec_destroy(CVec*);
int cvec_is_empty(CVec*);

#endif // CVEC_H
