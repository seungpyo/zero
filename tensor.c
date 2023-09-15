#include <stdbool.h>
#include <math.h>
#include "tensor.h"

static const char* zero_dtype_name(enum zero_dtype t) {
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
    default:
        fprintf(stderr, "unknown dtype: %d\n", t);
        return NULL;
        break;
    }
}

static size_t zero_dtype_size(enum zero_dtype t) {
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

void zero_tensor_print(struct zero_tensor *t) {
    fprintf(stderr, "Tensor: \tname=%s\tshape=[", t->name);
    for (int i = 0; i < t->ndim; i++) {
        fprintf(stderr, "%d", t->shape[i]);
        if (i < t->ndim - 1) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "]\tdtype=%s\n", zero_dtype_name(t->dtype));
}

int zero_tensor_fill(struct zero_tensor *t, float value) {
    size_t numel = zero_tensor_numel(t);
    if (t->data == NULL) {
        return -1;
    }
    for (int i = 0; i < numel; i++) {
        ((float *)t->data)[i] = value;
    }
    return 0;
}

int zero_tensor_save(FILE* fp, struct zero_tensor *t) {
    if (t == NULL) {
        fprintf(stderr, "tensor is NULL\n");
        return -1;
    }
    fwrite(t->name, sizeof(char), ZERO_MAX_TENSOR_NAME_LEN, fp);
    fwrite(&t->dtype, sizeof(enum zero_dtype), 1, fp);
    fwrite(&t->ndim, sizeof(int), 1, fp);
    fwrite(t->shape, sizeof(int), t->ndim, fp);
    fwrite(t->data, zero_dtype_size(t->dtype), zero_tensor_numel(t), fp);
    return 0;
}

int zero_tensor_load(FILE *fp, struct zero_tensor *t) {
    if (t == NULL) {
        fprintf(stderr, "tensor is NULL\n");
        return -1;
    }
    fread(t->name, sizeof(char), ZERO_MAX_TENSOR_NAME_LEN, fp);
    fread(&t->dtype, sizeof(enum zero_dtype), 1, fp);
    fread(&t->ndim, sizeof(int), 1, fp);
    t->shape = (int *)malloc(t->ndim * sizeof(int));
    fread(t->shape, sizeof(int), t->ndim, fp);
    t->data = (void *)malloc(zero_tensor_nbytes(t));
    fread(t->data, zero_dtype_size(t->dtype), zero_tensor_numel(t), fp);
    return 0;
}

bool zero_tensor_equals(struct zero_tensor *lhs, struct zero_tensor *rhs, float eps) {
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
    for (int i = 0; i < numel; i++) {
        if (fabs(((float *)lhs->data)[i] - ((float *)rhs->data)[i]) > eps) {
            return false;
        }
    }
    return true;
}

int main() {
    int *shape = (int *)malloc(2 * sizeof(int));
    shape[0] = 2;
    shape[1] = 3;
    struct zero_tensor x;
    zero_tensor_init(&x, "test", ZERO_FLOAT32, 2, shape);
    zero_tensor_fill(&x, 3.14f);
    zero_tensor_print(&x);
    
    FILE *fp = fopen("test.tensor", "wb");
    zero_tensor_save(fp, &x);
    fclose(fp);

    struct zero_tensor y;
    fp = fopen("test.tensor", "rb");
    zero_tensor_load(fp, &y);
    fclose(fp);
    
    bool ok = zero_tensor_equals(&x, &y, 1e-6);

    if (ok) {
        fprintf(stderr, "test passed\n");
    } else {
        fprintf(stderr, "test failed\n");
    }

    zero_tensor_free(&x);
    zero_tensor_free(&y);
    free(shape);
    return 0;
}