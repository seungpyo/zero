#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

enum zero_blob_type {
    ZERO_BLOB_BYTES,
    ZERO_BLOB_TENSOR,
    ZERO_BLOB_OPERATOR
};

struct zero_blob {
    size_t(*size)(struct blob *self);
    void(*free)(struct blob *self);
    void(*save)(struct blob *self, FILE *fp);
    void(*load)(struct blob *self, FILE *fp);
    enum zero_blob_type type;
    void *data;
};


