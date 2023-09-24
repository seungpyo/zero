#ifndef _ZERO_TENSOR_H
#define _ZERO_TENSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define ZERO_MAX_TENSOR_NAME_LEN 128

enum zero_dtype {
    ZERO_FLOAT32,
    ZERO_INT32,
    ZERO_INT64,
    ZERO_BFLOAT16
};
const char* zero_dtype_name(enum zero_dtype t);
size_t zero_dtype_size(enum zero_dtype t);

inline float zero_bfloat16_to_float(uint16_t b);
inline uint16_t zero_float_to_bfloat16(float x);


struct zero_tensor {
    char name[ZERO_MAX_TENSOR_NAME_LEN];
    enum zero_dtype dtype;
    int ndim;
    int *shape;
    void *data;
};
size_t zero_tensor_numel(struct zero_tensor *t);
void zero_tensor_init(struct zero_tensor *t, char *name, enum zero_dtype dtype, int ndim, int *shape);
void zero_tensor_free(struct zero_tensor *t);
void zero_tensor_print(struct zero_tensor *t);
int zero_tensor_fill(struct zero_tensor *t, void *value);
bool zero_tensor_equals(struct zero_tensor *lhs, struct zero_tensor *rhs, float eps, float *max_diff);

#endif // _ZERO_TENSOR_H