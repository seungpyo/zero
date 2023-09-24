
#include "ops.h"


void zero_ops_load_4_bfloat16(uint64_t *src, float *dst) {
    dst[0] = zero_bfloat16_to_float(src[0]);
    dst[1] = zero_bfloat16_to_float(src[1]);
    dst[2] = zero_bfloat16_to_float(src[2]);
    dst[3] = zero_bfloat16_to_float(src[3]);
}

void zero_ops_save_4_bfloat16(float *src, uint64_t *dst) {
    uint16_t *ptr = (uint16_t *)dst;
    ptr[0] = zero_float_to_bfloat16(src[0]);
    ptr[1] = zero_float_to_bfloat16(src[1]);
    ptr[2] = zero_float_to_bfloat16(src[2]);
    ptr[3] = zero_float_to_bfloat16(src[3]);
}


void zero_ops_fill(struct zero_tensor *t, float value) {
    int numel = zero_tensor_numel(t);
    uint16_t b;
    switch (t->dtype) {
        case ZERO_FLOAT32:
            for (int i = 0; i < numel; ++i) {
                ((float *)t->data)[i] = value;
            }
            break;
        case ZERO_INT32:
            for (int i = 0; i < numel; ++i) {
                ((int *)t->data)[i] = value;
            }
            break;
        case ZERO_INT64:
            for (int i = 0; i < numel; ++i) {
                ((long *)t->data)[i] = value;
            }
            break;
        case ZERO_BFLOAT16:
            b = zero_float_to_bfloat16(value);
            for (int i = 0; i < numel; ++i) {
                ((uint16_t *)t->data)[i] = b;
            }
            break;
        default:
            fprintf(stderr, "[ZERO] Error: fill does not support dtype %d\n", t->dtype);
            exit(1);
    }
}

void zero_ops_add(struct zero_tensor *lhs, struct zero_tensor *rhs, struct zero_tensor *res) {
    #ifdef ZERO_VALIDATE_OPERANDS
    zero_ops_check_same_dtype(lhs, rhs);
    zero_ops_check_same_ndim(lhs, rhs);
    zero_ops_check_same_shape(lhs, rhs);
    zero_ops_check_same_dtype(lhs, res);
    zero_ops_check_same_ndim(lhs, res);
    zero_ops_check_same_shape(lhs, res);
    #endif // ZERO_VALIDATE_OPERANDS

    float lhs_f, rhs_f, res_f;
    float lhs_4f[4], rhs_4f[4], res_4f[4];
    int numel = zero_tensor_numel(lhs);
    switch (lhs->dtype) {
        case ZERO_FLOAT32:
            for (int i = 0; i < numel; ++i) {
                ((float *)res->data)[i] = ((float *)lhs->data)[i] + ((float *)rhs->data)[i];
            }
            break;
        case ZERO_INT32:
            for (int i = 0; i < numel; ++i) {
                ((int *)res->data)[i] = ((int *)lhs->data)[i] + ((int *)rhs->data)[i];
            }
            break;
        case ZERO_INT64:
            for (int i = 0; i < numel; ++i) {
                ((long *)res->data)[i] = ((long *)lhs->data)[i] + ((long *)rhs->data)[i];
            }
            break;
        case ZERO_BFLOAT16:
            for (int i = 0; i < numel / 4; ++i) {
                zero_ops_load_4_bfloat16((uint64_t *)lhs->data + i, lhs_4f);
                zero_ops_load_4_bfloat16((uint64_t *)rhs->data + i, rhs_4f);
                res_4f[0] = lhs_4f[0] + rhs_4f[0];
                res_4f[1] = lhs_4f[1] + rhs_4f[1];
                res_4f[2] = lhs_4f[2] + rhs_4f[2];
                res_4f[3] = lhs_4f[3] + rhs_4f[3];
                zero_ops_save_4_bfloat16(res_4f, (uint64_t *)res->data + i);
            }
            break;
        default:
            fprintf(stderr, "[ZERO] Error: add does not support dtype %d\n", lhs->dtype);
            exit(1);
    }
}

void zero_ops_scale(struct zero_tensor *t, float scale, struct zero_tensor *res) {
    #ifdef ZERO_VALIDATE_OPERANDS
    zero_ops_check_same_dtype(lhs, res);
    zero_ops_check_same_ndim(lhs, res);
    zero_ops_check_same_shape(lhs, res);
    #endif // ZERO_VALIDATE_OPERANDS

    int numel = zero_tensor_numel(t);
    float t_f, res_f;
    float t_4f[4], res_4f[4];
    switch (t->dtype) {
        case ZERO_FLOAT32:
            for (int i = 0; i < numel; ++i) {
                ((float *)res->data)[i] = ((float *)t->data)[i] * scale;
            }
            break;
        case ZERO_INT32:
            for (int i = 0; i < numel; ++i) {
                ((int *)res->data)[i] = ((int *)t->data)[i] * scale;
            }
            break;
        case ZERO_INT64:
            for (int i = 0; i < numel; ++i) {
                ((long *)res->data)[i] = ((long *)t->data)[i] * scale;
            }
            break;
        case ZERO_BFLOAT16:
            for (int i = 0; i < numel / 4; ++i) {
                zero_ops_load_4_bfloat16((uint64_t *)t->data + i, t_4f);
                res_4f[0] = t_4f[0] * scale;
                res_4f[1] = t_4f[1] * scale;
                res_4f[2] = t_4f[2] * scale;
                res_4f[3] = t_4f[3] * scale;
                zero_ops_save_4_bfloat16(res_4f, (uint64_t *)res->data + i);
            }
            break;

    }
}

void zero_ops_matmul(struct zero_tensor *lhs, struct zero_tensor *rhs, struct zero_tensor *res, int block_size) {
    #ifdef ZERO_VALIDATE_OPERANDS
    if (lhs->ndim != 2 || rhs->ndim != 2 || res->ndim != 2) {
        fprintf(stderr, "[ZERO] Error: matmul requires 2D tensors\n");
        exit(1);
    }
    zero_ops_check_same_dtype(lhs, rhs);
    zero_ops_check_same_dtype(lhs, res);
    if (lhs->shape[1] != rhs->shape[0] || lhs->shape[0] != res->shape[0] || rhs->shape[1] != res->shape[1]) {
        fprintf(stderr, "[ZERO] Error: matmul on invalid shape (%d x %d) @ (%d x %d) -> (%d x %d)\n", lhs->shape[0], lhs->shape[1], rhs->shape[0], rhs->shape[1], res->shape[0], res->shape[1]);
        exit(1);
    }
    #endif // ZERO_VALIDATE_OPERANDS
    int i, j, k, m = lhs->shape[0], n = rhs->shape[1], p = lhs->shape[1];
    switch (res->dtype) {
        case ZERO_FLOAT32:
            for (i = 0; i < m; i += block_size) {
                for (j = 0; j < n; j += block_size) {
                    for (k = 0; k < p; k += block_size) {
                        for (int ii = i; ii < i + block_size; ++ii) {
                            for (int jj = j; jj < j + block_size; ++jj) {
                                float sum = 0.0;
                                for (int kk = k; kk < k + block_size; ++kk) {
                                    sum += ((float *)lhs->data)[ii * m + kk] * ((float *)rhs->data)[kk * p + jj];
                                }
                                ((float *)res->data)[ii * n + jj] += sum;
                            }
                        }
                    }
                }
            }
            break;
        case ZERO_INT32:
            for (i = 0; i < m; i += block_size) {
                for (j = 0; j < n; j += block_size) {
                    for (k = 0; k < p; k += block_size) {
                        for (int ii = i; ii < i + block_size; ++ii) {
                            for (int jj = j; jj < j + block_size; ++jj) {
                                int sum = 0;
                                for (int kk = k; kk < k + block_size; ++kk) {
                                    sum += ((int *)lhs->data)[ii * m + kk] * ((int *)rhs->data)[kk * p + jj];
                                }
                                ((int *)res->data)[ii * n + jj] += sum;
                            }
                        }
                    }
                }
            }
            break;
        case ZERO_INT64:
            for (i = 0; i < m; i += block_size) {
                for (j = 0; j < n; j += block_size) {
                    for (k = 0; k < p; k += block_size) {
                        for (int ii = i; ii < i + block_size; ++ii) {
                            for (int jj = j; jj < j + block_size; ++jj) {
                                long sum = 0;
                                for (int kk = k; kk < k + block_size; ++kk) {
                                    sum += ((long *)lhs->data)[ii * m + kk] * ((long *)rhs->data)[kk * p + jj];
                                }
                                ((long *)res->data)[ii * n + jj] += sum;
                            }
                        }
                    }
                }
            }
            break;
        case ZERO_BFLOAT16:
            for (i = 0; i < m; i += block_size * 4) {
                for (j = 0; j < n; j += block_size * 4) {
                    for (k = 0; k < p; k += block_size * 4) {
                        for (int ii = i; ii < i + block_size; ++i) {
                            for (int jj = j; jj < j + block_size; ++j) {
                                float sum = 0.0;
                                for (int kk = k; kk < k + block_size * 4; kk += 4) {
                                    float lhs_4f[4], rhs_4f[4];
                                    zero_ops_load_4_bfloat16((uint64_t *)lhs->data + ii * m + kk, lhs_4f);
                                    zero_ops_load_4_bfloat16((uint64_t *)rhs->data + kk * p + jj, rhs_4f);
                                    sum += lhs_4f[0] * rhs_4f[0];
                                    sum += lhs_4f[1] * rhs_4f[1];
                                    sum += lhs_4f[2] * rhs_4f[2];
                                    sum += lhs_4f[3] * rhs_4f[3];
                                }
                                ((float *)res->data)[ii * m + jj] += sum;
                            }
                        }
                    }
                }
            }
            break;
    }
}
