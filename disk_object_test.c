#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "tensor.h"
#include "disk_object.h"

int main() {
    const int num_tensors = 10;
    struct zero_tensor *tensors = malloc(sizeof(struct zero_tensor) * num_tensors);
    float val;
    for (int i = 0; i < num_tensors; ++i) {
        char name[ZERO_MAX_TENSOR_NAME_LEN];
        sprintf(name, "tensor_%d", i);
        zero_tensor_init(&tensors[i], name, ZERO_BFLOAT16, 2, (uint32_t[]){1 << (i + 1), 1 << (i + 1)});
        val = (float)i;
        zero_tensor_fill(&tensors[i], &val);
    }
    struct zero_disk_object *objs = malloc(sizeof(struct zero_disk_object) * num_tensors);
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_serialize_tensor(&objs[i], &tensors[i]);
    }
    FILE *fp = fopen("test.zero", "wb");
    uint64_t offset = (sizeof(struct zero_disk_object) - sizeof(void *)) * num_tensors;
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_write_header(&objs[i], fp);
        offset += objs[i].size;
    }
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_write_data(&objs[i], fp);
    }
    fclose(fp);

    fp = fopen("test.zero", "rb");
    struct zero_tensor *tensors2 = malloc(sizeof(struct zero_tensor) * num_tensors);
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_read_header(&objs[i], fp);
    }
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_read_data(&objs[i], fp);
    }
    fclose(fp);
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_deserialize_tensor(&objs[i], &tensors2[i]);
    }
    float max_diff = 0.0f;
    for (int i = 0; i < num_tensors; ++i) {
       if (!zero_tensor_equals(&tensors[i], &tensors2[i], 1e-5, &max_diff)) {
           printf("Tensor %d not equal\n", i);
           printf("max_diff: %f\n", max_diff);
           return 1;
       } else {
        printf("Tensor %d equal (max_diff = %f)\n", i, max_diff);
       }
    }
    printf("All tensors equal\n");
    return 0;
}

