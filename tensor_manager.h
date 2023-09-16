#include "tensor.h"
#define ZERO_MAX_TENSOR_MANGER_FILENAME_LEN 128
struct zero_tensor_allocation {
    uint32_t name_hash;
    struct zero_tensor *tensor;
    struct zero_tensor_allocation *next;
};

struct zero_tensor_manager {
    char filename[ZERO_MAX_TENSOR_MANGER_FILENAME_LEN];
    uint32_t num_tensors;
    struct zero_tensor_allocation *allocations;
};

void *zero_tensor_manager_init(struct zero_tensor_manager *m, const char *filename);
void zero_tensor_manager_free(struct zero_tensor_manager *m);
void zero_tensor_manager_print(struct zero_tensor_manager *m);
void zero_tensor_manager_save(struct zero_tensor_manager *m);
void zero_tensor_manager_load(struct zero_tensor_manager *m);
void zero_tensor_manager_add(struct zero_tensor_manager *m, struct zero_tensor *t);
struct zero_tensor *zero_tensor_manager_get(struct zero_tensor_manager *m, char *name);
void zero_tensor_manager_remove(struct zero_tensor_manager *m, char *name);
void zero_tensor_manager_remove_all(struct zero_tensor_manager *m);
