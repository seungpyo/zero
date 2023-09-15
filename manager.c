#include "tensor.h"
#include "manager.h"

void zero_tensor_manager_init(struct zero_tensor_manager *manager, char *savepath) {
    manager->savepath = (char *)malloc(strlen(savepath) * sizeof(char));
    strncpy(manager->savepath, savepath, strlen(savepath));
    manager->tensor_offset = 0;
    manager->tensor_list = NULL;
}

void zero_tensor_manager_free(struct zero_tensor_manager *manager) {
    free(manager->savepath);
    struct zero_tensor_list *p = manager->tensor_list;
    while (p != NULL) {
        struct zero_tensor_list *q = p;
        p = p->next;
        zero_tensor_free(q->tensor);
        free(q);
    }
}

uint32_t fnv_hash(const void* s) {
    uint32_t hash = 2166136261;
    uint8_t *p;
    for(p = (uint8_t *)s; *p; ++p) hash = (hash ^ *p) * 16777619;
    return hash;
}

int zero_tensor_manager_add(
    struct zero_tensor_manager *manager, char *name, enum zero_dtype dtype, int ndim, int *shape) {
    struct zero_tensor_list *p = (struct zero_tensor_list *)malloc(sizeof(struct zero_tensor_list));
    struct zero_tensor *tensor = (struct zero_tensor *)malloc(sizeof(struct zero_tensor));
    zero_tensor_init(tensor, name, dtype, ndim, shape);
    p->tensor = tensor;
    p->next = manager->tensor_list;
    manager->tensor_list = p;
    manager->tensor_offset += zero_tensor_nbytes(tensor);
}

void zero_tensor_manager_print(struct zero_tensor_manager *manager) {
    fprintf(stderr, "Zero Tensor Manager(savepath=%s, tensor_offset=%lu)\n", manager->savepath, manager->tensor_offset);
    fprintf(stderr, "Total memory usage: %lu bytes\n", manager->tensor_offset);
    fprintf(stderr, "Tensor List:\n");
    struct zero_tensor_list *p = manager->tensor_list;
    while (p != NULL) {
        zero_tensor_print(p->tensor);
        p = p->next;
    }
}


int zero_tensor_manger_save(struct zero_tensor_manager *manager) {
    FILE *fp = fopen(manager->savepath, "wb");
    if (fp == NULL) {
        fprintf(stderr, "failed to open file: %s\n", manager->savepath);
        return -1;
    }
    struct zero_tensor_list *p = manager->tensor_list;
    while (p != NULL) {
        zero_tensor_save(fp, p->tensor);
        p = p->next;
    }
    fclose(fp);
    return 0;
}

void zero_manager_test_duplicate_add() {
    struct zero_tensor_manager manager;
    zero_tensor_manager_init(&manager, "test.zero");
    int *shape = (int *)malloc(2 * sizeof(int));
    shape[0] = 2;
    shape[1] = 3;
    zero_tensor_manager_add(&manager, "dup", ZERO_FLOAT32, 2, shape);
    zero_tensor_manager_add(&manager, "dup", ZERO_FLOAT32, 2, shape);
}


int main() {
    struct zero_tensor t;
    struct zero_tensor_manager manager;
    zero_tensor_manager_init(&manager, "test.zero");
    int *shape = (int *)malloc(2 * sizeof(int));
    shape[0] = 2;
    shape[1] = 3;
    zero_tensor_manager_add(&manager, "dup", ZERO_FLOAT32, 2, shape);
    zero_tensor_manager_add(&manager, "dup", ZERO_FLOAT32, 2, shape);
    for (int i = 0; i < 10; ++i) {
        zero_tensor_manager_print(&manager);
        zero_tensor_manager_add(&manager, "test", ZERO_FLOAT32, 2, shape);
    }
    zero_tensor_manager_print(&manager);
    zero_tensor_manager_free(&manager);
    free(shape);
    return 0;
}