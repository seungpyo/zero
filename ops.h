#ifndef _ZERO_OPS_H
#define _ZERO_OPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tensor.h"

// #define ZERO_VALIDATE_OPERANDS

inline void zero_ops_load_4_bfloat16(uint64_t *src, float *dst);
// void zero_ops_load_4_bfloat16(uint64_t *src, float *dst) {
//     dst[0] = zero_bfloat16_to_float(src[0]);
//     dst[1] = zero_bfloat16_to_float(src[1]);
//     dst[2] = zero_bfloat16_to_float(src[2]);
//     dst[3] = zero_bfloat16_to_float(src[3]);
// }

inline void zero_ops_save_4_bfloat16(float *src, uint64_t *dst);
// void zero_ops_save_4_bfloat16(float *src, uint64_t *dst) {
//     uint16_t *ptr = (uint16_t *)dst;
//     ptr[0] = zero_float_to_bfloat16(src[0]);
//     ptr[1] = zero_float_to_bfloat16(src[1]);
//     ptr[2] = zero_float_to_bfloat16(src[2]);
//     ptr[3] = zero_float_to_bfloat16(src[3]);
// }

inline void zero_ops_check_same_dtype(struct zero_tensor *lhs, struct zero_tensor *rhs) {
    if (lhs->dtype != rhs->dtype) {
        const char* lhs_dtype = zero_dtype_name(lhs->dtype);
        const char* rhs_dtype = zero_dtype_name(rhs->dtype);
        fprintf(stderr, "[ZERO] Error: Cannot operate on lhs(ndim=%d, dtype=%s) and rhs(ndim=%d, dtype=%s)\n", lhs->ndim, lhs_dtype, rhs->ndim, rhs_dtype);
        exit(1);
    }
}

inline void zero_ops_check_same_ndim(struct zero_tensor *lhs, struct zero_tensor *rhs) {
    if (lhs->ndim != rhs->ndim) {
        fprintf(stderr, "[ZERO] Error: Cannot operate on lhs(ndim=%d) and rhs(ndim=%d)\n", lhs->ndim, rhs->ndim);
        exit(1);
    }
}

inline void zero_ops_check_same_shape(struct zero_tensor *lhs, struct zero_tensor *rhs) {
    for (int i = 0; i < lhs->ndim; i++) {
        if (lhs->shape[i] != rhs->shape[i]) {
            fprintf(stderr, "[ZERO] Error: Cannot operate on lhs(shape[%d]=%d) and rhs(shape[%d]=%d)\n", i, lhs->shape[i], i, rhs->shape[i]);
            exit(1);
        }
    }
}

void zero_ops_fill(struct zero_tensor *t, float value);
void zero_ops_add(struct zero_tensor *lhs, struct zero_tensor *rhs, struct zero_tensor *res);
void zero_ops_scale(struct zero_tensor *t, float scale, struct zero_tensor *res);
void zero_ops_matmul(struct zero_tensor *lhs, struct zero_tensor *rhs, struct zero_tensor *res, int block_size);

#endif // _ZERO_OPS_H