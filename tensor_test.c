#include "tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>



int main() {
    int *shape = (int *)malloc(2 * sizeof(int));
    shape[0] = 2;
    shape[1] = 3;
    struct zero_tensor x;
    enum zero_dtype dtype = ZERO_BFLOAT16;
    zero_tensor_init(&x, "test", dtype, 2, shape);
    float val = 3.14f;
    zero_tensor_fill(&x, &val);
    FILE *fp = fopen("test.tensor", "wb");
    zero_tensor_save(fp, &x);
    fclose(fp);

    struct zero_tensor y;
    zero_tensor_init(&y, "test", dtype, 2, shape);
    fp = fopen("test.tensor", "rb");
    zero_tensor_load(fp, &y);
    fclose(fp);

    float max_diff;
    if (zero_tensor_equals(&x, &y, 1e-6f, &max_diff)) {
        fprintf(stderr, "OK: x and y are equal\n");
    } else {
        fprintf(stderr, "FAIL: x and y are not equal\n");
    }

    zero_tensor_print(&x);
    zero_tensor_free(&x);
    free(shape);
    return 0;
}