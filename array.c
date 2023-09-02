#include "array.h"

void *array_alloc_data(array *a) {
    return (void *)malloc(a->capacity * a->elem_size);
}

array *array_alloc(size_t elem_size, size_t capacity) {
    array *a = (array *)malloc(sizeof(array));
    a->size = 0;
    a->capacity = capacity;
    a->elem_size = elem_size;
    a->data = array_alloc_data(a);
    return a;
}

void array_free(array *a) {
    if (a->data) {
        free(a->data);
    }
    free(a);
}


array *array_slice(array *src, int from, int to) {
    if (from < 0 || to > src->size) {
        return NULL;
    }
    array *dst = array_alloc(src->elem_size, to - from);
    memcpy(dst->data, src->data, dst->elem_size * dst->size);
    return dst;
}

array *array_copy(array *src) {
    return array_slice(src, 0, src->size);
}

int array_resize(array *src, size_t capacity) {
    if (capacity < 0 || src == NULL) {
        return -1;
    }
    if (src->capacity == capacity) {
        return 0;
    }
    void *buf = (void *)malloc(src->elem_size * capacity);
    size_t min_cap = src->capacity < capacity ? src->capacity : capacity;
    memcpy(buf, src->data, min_cap * src->elem_size);
    free(src->data);
    src->data = buf;
    src->capacity = capacity;
    src->size = src->size < capacity ? src->size: capacity;
    return 0;
}

int array_append(array *src, void *data) {
    if (src == NULL || data == NULL) {
        return -1;
    }
    if (src->size >= src->capacity) {
        array_resize(src, src->capacity * 2);
    }
    memcpy((char *)src->data + src->elem_size * src->size, data, src->elem_size);
    src->size++;
    return 0;
}

void * array_at(array *a, int idx) {
    return (void *)((char *)a->data + idx * a->elem_size);
}

void array_pprint(array *a, char *format) {
    printf("array(size=%lu, capacity=%lu, elem_size=%lu)\n[", a->size, a->capacity, a->elem_size);
    for (int i = 0; i < a->size; ++i) {
        char tmp[16];
        printf(tmp, format, (char *)array_at(a, i));
        if (i + 1 < a->size) {
        printf(", ");
        }
    }
    printf("]\n");
}

// int main() {
//     array *a = array_alloc(sizeof(char *), 100);
//     for (int i = 0; i < a->capacity; ++i) {
//         char msg[16];
//         sprintf(msg, "hi- %d", i);
//         array_append(a, &msg);
//     }
//     char msg[4096];
//     array_pprint(a, msg, "%s");
//     printf("%s", msg);
//     array_free(a);
// }