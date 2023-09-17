#include <stdbool.h>
#include <math.h>
#include "tensor.h"


float bfloat16_to_float(uint16_t b) {
    uint32_t tmp = ((uint32_t)b) << 16;
    return *((float *)&tmp);
}

uint16_t float_to_bfloat16(float x) {
    uint16_t b = (uint16_t)(*((uint32_t *)&x) >> 16);
    return b;
}



const char* zero_dtype_name(enum zero_dtype t) {
    switch (t)
    {
    case ZERO_FLOAT32:
        return "float32";
        break;
    case ZERO_INT32:
        return "int32";
        break;
    case ZERO_INT64:
        return "int64";
        break;
    case ZERO_BFLOAT16:
        return "bfloat16";
        break;
    default:
        fprintf(stderr, "unknown dtype: %d\n", t);
        return NULL;
        break;
    }
}

size_t zero_dtype_size(enum zero_dtype t) {
    switch (t)
    {
    case ZERO_FLOAT32:
        return sizeof(float);
        break;
    case ZERO_INT32:
        return sizeof(int32_t);
        break;
    case ZERO_INT64:
        return sizeof(int64_t);
        break;
    case ZERO_BFLOAT16:
        return sizeof(uint16_t);
        break;
    default:
        fprintf(stderr, "unknown dtype: %d\n", t);
        return -1;
        break;
    }
}
size_t zero_tensor_numel(struct zero_tensor *t) {
    size_t numel = 1;
    for (int i = 0; i < t->ndim; i++) {
        numel *= t->shape[i];
    }
    return numel;
}
size_t zero_tensor_nbytes(struct zero_tensor *t) {
    return zero_tensor_numel(t) * zero_dtype_size(t->dtype);
}


void zero_tensor_init(
    struct zero_tensor *t, 
    char *name, enum zero_dtype dtype, int ndim, int *shape) {
    memset(t, 0, sizeof(struct zero_tensor));
    strncpy(t->name, name, ZERO_MAX_TENSOR_NAME_LEN);
    t->dtype = dtype;
    t->ndim = ndim;
    t->shape = (int *)malloc(ndim * sizeof(int));
    memcpy(t->shape, shape, ndim * sizeof(int));
    t->data = (void *)malloc(zero_tensor_nbytes(t));
}

void zero_tensor_free(struct zero_tensor *t) {
    free(t->shape);
    free(t->data);
}

#define ZERO_TENSOR_PRINT_DATA

void zero_tensor_print(struct zero_tensor *t) {
    fprintf(stderr, "Tensor: \tname=%s\tshape=[", t->name);
    for (int i = 0; i < t->ndim; i++) {
        fprintf(stderr, "%d", t->shape[i]);
        if (i < t->ndim - 1) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "]\tdtype=%s\n", zero_dtype_name(t->dtype));
    #ifdef ZERO_TENSOR_PRINT_DATA
    size_t numel = zero_tensor_numel(t);
    if (t->dtype == ZERO_FLOAT32) {
        for (int i = 0; i < numel; i++) {
            fprintf(stderr, "%f", ((float *)t->data)[i]);
            if (i < numel - 1) {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "\n");
    } else if (t->dtype == ZERO_INT32) {
        for (int i = 0; i < numel; i++) {
            fprintf(stderr, "%d", ((int32_t *)t->data)[i]);
            if (i < numel - 1) {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "\n");
    } else if (t->dtype == ZERO_INT64) {
        for (int i = 0; i < numel; i++) {
            fprintf(stderr, "%lld", ((int64_t *)t->data)[i]);
            if (i < numel - 1) {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "\n");
    } else if (t->dtype == ZERO_BFLOAT16) {
        for (int i = 0; i < numel; i++) {
            fprintf(stderr, "%f", bfloat16_to_float(((uint16_t *)t->data)[i]));
            if (i < numel - 1) {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "\n");
    } else {
        fprintf(stderr, "unknown dtype: %d\n", t->dtype);
    }
    #endif
}

int zero_tensor_fill(struct zero_tensor *t, void* value) {
    size_t numel = zero_tensor_numel(t);
    if (t->data == NULL) {
        return -1;
    }
    for (int i = 0; i < numel; i++) {
        if (t->dtype == ZERO_FLOAT32) {
            ((float *)t->data)[i] = *((float *)value);
        } else if (t->dtype == ZERO_INT32) {
            ((int32_t *)t->data)[i] = *((int32_t *)value);
        } else if (t->dtype == ZERO_INT64) {
            ((int64_t *)t->data)[i] = *((int64_t *)value);
        } else if (t->dtype == ZERO_BFLOAT16) {
            ((uint16_t *)t->data)[i] = float_to_bfloat16(*((float *)value));
        } else {
            fprintf(stderr, "unknown dtype: %d\n", t->dtype);
            return -1;
        }
    }
    return 0;
}

size_t zero_tensor_save(FILE *fp, struct zero_tensor *t) {
    size_t offset0 = ftell(fp);
    fwrite(t->name, sizeof(char), ZERO_MAX_TENSOR_NAME_LEN, fp);
    fwrite(&(t->dtype), sizeof(enum zero_dtype), 1, fp);
    fwrite(&(t->ndim), sizeof(int), 1, fp);
    fwrite(t->shape, sizeof(int), t->ndim, fp);
    fwrite(t->data, zero_dtype_size(t->dtype), zero_tensor_numel(t), fp);
    size_t offset1 = ftell(fp);
    return offset1 - offset0;
}

size_t zero_tensor_load(FILE *fp, struct zero_tensor *t) {
    size_t offset0 = ftell(fp);
    fread(t->name, sizeof(char), ZERO_MAX_TENSOR_NAME_LEN, fp);
    fread(&(t->dtype), sizeof(enum zero_dtype), 1, fp);
    fread(&(t->ndim), sizeof(int), 1, fp);
    t->shape = (int *)malloc(t->ndim * sizeof(int));
    fread(t->shape, sizeof(int), t->ndim, fp);
    t->data = (void *)malloc(zero_tensor_nbytes(t));
    fread(t->data, zero_dtype_size(t->dtype), zero_tensor_numel(t), fp);
    size_t offset1 = ftell(fp);
    return offset1 - offset0;
}

bool zero_tensor_equals(struct zero_tensor *lhs, struct zero_tensor *rhs, float eps, float *max_diff) {
    if (lhs->dtype != rhs->dtype) {
        return false;
    }
    if (lhs->ndim != rhs->ndim) {
        return false;
    }
    for (int i = 0; i < lhs->ndim; i++) {
        if (lhs->shape[i] != rhs->shape[i]) {
            return false;
        }
    }
    size_t numel = zero_tensor_numel(lhs);
    float max_diff_val = 0.0f;
    switch(lhs->dtype) {
        case ZERO_FLOAT32:
        for (int i = 0; i < numel; i++) {
            max_diff_val = fmax(max_diff_val, fabs(((float *)lhs->data)[i] - ((float *)rhs->data)[i]));
            if (max_diff_val > eps) {
                if (max_diff != NULL) {
                    *max_diff = max_diff_val;
                }
                return false;
            }
        }
        *max_diff = max_diff_val;
        return true;
        break;
        case ZERO_INT32:
        for (int i = 0; i < numel; i++) {
            if (((int32_t *)lhs->data)[i] != ((int32_t *)rhs->data)[i]) {
                return false;
            }
        }
        return true;
        break;
        case ZERO_INT64:
        for (int i = 0; i < numel; i++) {
            if (((int64_t *)lhs->data)[i] != ((int64_t *)rhs->data)[i]) {
                return false;
            }
        }
        return true;
        break;
        case ZERO_BFLOAT16:
        for (int i = 0; i < numel; i++) {
            float lhs_f = bfloat16_to_float(((uint16_t *)lhs->data)[i]);
            float rhs_f = bfloat16_to_float(((uint16_t *)rhs->data)[i]);
            if (fabs(lhs_f - rhs_f) > eps) {
                return false;
            }
        }
        return true;
        break;
        default:
        fprintf(stderr, "unknown dtype: %d\n", lhs->dtype);
        return false;
        break;
    }
}

// int main() {
//     int *shape = (int *)malloc(2 * sizeof(int));
//     shape[0] = 2;
//     shape[1] = 3;
//     struct zero_tensor x;
//     enum zero_dtype dtype = ZERO_BFLOAT16;
//     zero_tensor_init(&x, "test", dtype, 2, shape);
//     float val = 3.14f;
//     zero_tensor_fill(&x, &val);
//     FILE *fp = fopen("test.tensor", "wb");
//     zero_tensor_save(fp, &x);
//     fclose(fp);

//     struct zero_tensor y;
//     zero_tensor_init(&y, "test", dtype, 2, shape);
//     fp = fopen("test.tensor", "rb");
//     zero_tensor_load(fp, &y);
//     fclose(fp);

//     float max_diff;
//     if (zero_tensor_equals(&x, &y, 1e-6f, &max_diff)) {
//         fprintf(stderr, "OK: x and y are equal\n");
//     } else {
//         fprintf(stderr, "FAIL: x and y are not equal\n");
//     }

//     zero_tensor_print(&x);
//     zero_tensor_free(&x);
//     free(shape);
//     return 0;
// }