#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "tensor.h"
#include "disk_object.h"

int main() {
    FILE *fp = fopen("capi.zero", "rb");
    int num_tensors;
    fread(&num_tensors, sizeof(int), 1, fp);
    struct zero_disk_object *objs = malloc(sizeof(struct zero_disk_object) * num_tensors);
    struct zero_tensor *tensors = malloc(sizeof(struct zero_tensor) * num_tensors);
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_read_header(&objs[i], fp);
        objs[i].data = malloc(objs[i].size);
    }
    for (int i = 0; i < num_tensors; ++i) {
        zero_disk_object_read_data(&objs[i], fp);
        zero_disk_object_deserialize_tensor(&objs[i], &tensors[i]);
    }
    fclose(fp);
    for (int i = 0; i < num_tensors; ++i) {
        printf("Tensor %d:\n", i);
        zero_tensor_print(&tensors[i]);
    }
    for (int i = 0; i < num_tensors; ++i) {
        free(objs[i].data);
        zero_tensor_free(&tensors[i]);
    }
    return 0;
}