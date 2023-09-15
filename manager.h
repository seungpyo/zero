#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tensor.h"

struct zero_tensor_list {
    struct zero_tensor *tensor;
    struct zero_tensor_list *next;
};
struct zero_tensor_manager {
    char *savepath;
    size_t tensor_offset;
    struct zero_tensor_list *tensor_list;
};

void zero_tensor_manager_init(struct zero_tensor_manager *manager, char *savepath);
void zero_tensor_manager_free(struct zero_tensor_manager *manager);
int zero_tensor_manager_add(struct zero_tensor_manager *manager, char *name, enum zero_dtype dtype, int ndim, int *shape);
void zero_tensor_manager_print(struct zero_tensor_manager *manager) ;
int zero_tensor_manger_save(FILE *fp, struct zero_tensor_manager *manager);
void zero_tensor_manager_load(FILE *fp, struct zero_tensor_manager *manager);
struct zero_tensor * zero_tensor_manager_get(struct zero_tensor_manager *manager, char *name);
