#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _array {
    void *data;
    size_t size;
    size_t elem_size;
    size_t capacity;
} array;

void *array_alloc_data(array *a);
array *array_alloc(size_t elem_size, size_t capacity);
void array_free(array *a);
array *array_slice(array *src, int from, int to);
array *array_copy(array *src);
int array_resize(array *src, size_t capacity);
int array_append(array *src, void *data);
void * array_at(array *a, int idx);
void array_pprint(array *a, char *format);
