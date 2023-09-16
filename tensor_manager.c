#include "tensor.h"
#define ZERO_MAX_TENSOR_MANGER_FILENAME_LEN 256
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


void *zero_tensor_manager_init(struct zero_tensor_manager *m, const char *filename) {
    memset(m, 0, sizeof(struct zero_tensor_manager));
    strncpy(m->filename, filename, ZERO_MAX_TENSOR_MANGER_FILENAME_LEN);
    m->num_tensors = 0;
    m->allocations = NULL;
    return m;
}
void zero_tensor_manager_free(struct zero_tensor_manager *m) {
    struct zero_tensor_allocation *a = m->allocations;
    while (a != NULL) {
        struct zero_tensor_allocation *next = a->next;
        zero_tensor_free(a->tensor);
        free(a->tensor);
        free(a);
        a = next;
    }
}
void zero_tensor_manager_print(struct zero_tensor_manager *m) {
    fprintf(stderr, "Tensor Manager: \tfilename=%s has %u tensors.\n", m->filename, m->num_tensors);
    struct zero_tensor_allocation *a = m->allocations;
    while (a != NULL) {
        zero_tensor_print(a->tensor);
        a = a->next;
    }
}
uint32_t zero_tensor_manager_fnv_hash(char *name) {
    uint32_t hash = 2166136261;
    for (int i = 0; i < strlen(name); i++) {
        hash = (hash * 16777619) ^ name[i];
    }
    return hash;
}

struct zero_tensor_manager_offset {
    uint32_t name_hash;
    uint32_t offset;
};

void zero_tensor_manager_save(struct zero_tensor_manager *m) {
    FILE *fp = fopen(m->filename, "wb");
    struct zero_tensor_allocation *a = m->allocations;
    // First of all, write a magic number
    fwrite("ZERO", sizeof(char), 4, fp);

    // Secondly, save (name_hash, disk offset) pairs  and its offset
    fwrite(&(m->num_tensors), sizeof(uint32_t), 1, fp);
    struct zero_tensor_manager_offset *offsets = (struct zero_tensor_manager_offset *)malloc(m->num_tensors * sizeof(struct zero_tensor_manager_offset));
    fseek(fp, m->num_tensors * sizeof(struct zero_tensor_manager_offset), SEEK_CUR);
    // Now dump all the tensors at the offset
    int i = 0;
    while (a != NULL) {
        offsets[i].name_hash = zero_tensor_manager_fnv_hash(a->tensor->name);
        offsets[i].offset = ftell(fp);
        zero_tensor_save(fp, a->tensor);
        a = a->next;
        i++;
    }
    // After dump, we can go back to write the offsets of each tensor
    // "ZERO" + sizeof(m->num_tensors) = 8
    fseek(fp, 8, SEEK_SET);
    fwrite(offsets, sizeof(struct zero_tensor_manager_offset), m->num_tensors, fp);
    fclose(fp);
}
void zero_tensor_manager_load(struct zero_tensor_manager *m) {
    // First of all, remove all the tensors to ensure that we have a clean start
    zero_tensor_manager_remove_all(m);

    FILE *fp = fopen(m->filename, "rb");
    // First of all, check the magic number
    char magic[4];
    fread(magic, sizeof(char), 4, fp);
    if (strncmp(magic, "ZERO", 4) != 0) {
        exit(1);
    }
    // Secondly, load (name_hash, disk offset) pairs
    uint32_t num_tensors;
    m->num_tensors = 0;
    fread(&(num_tensors), sizeof(uint32_t), 1, fp);
    fprintf(stderr, "num_tensors=%u\n", num_tensors);
    struct zero_tensor_manager_offset *offsets = (struct zero_tensor_manager_offset *)malloc(num_tensors * sizeof(struct zero_tensor_manager_offset));
    fread(offsets, sizeof(struct zero_tensor_manager_offset), num_tensors, fp);

    // Now load all the tensors at the offset
    // m->num_tensors is auto-incrmented in zero_tensor_manager_add
    for (int i = 0; i < num_tensors; i++) {
        fseek(fp, offsets[i].offset, SEEK_SET);
        struct zero_tensor *t = (struct zero_tensor *)malloc(sizeof(struct zero_tensor));
        zero_tensor_load(fp, t);
        zero_tensor_manager_add(m, t);
        fprintf(stderr, "After add, m->num_tensors=%u\n", m->num_tensors);
    }
    fclose(fp);
}
bool zero_tensor_manager_equals(struct zero_tensor_manager *lhs, struct zero_tensor_manager *rhs) {
    if (lhs->num_tensors != rhs->num_tensors) {
        fprintf(stderr, "Number of tensors does NOT match: %u != %u\n", lhs->num_tensors, rhs->num_tensors);
        return false;
    }
    struct zero_tensor_allocation *a = lhs->allocations;
    struct zero_tensor_allocation *b = rhs->allocations;
    float max_diff = 0.0f;
    while (a != NULL) {
        struct zero_tensor *t = zero_tensor_manager_get(rhs, a->tensor->name);
        if (t == NULL) {
            fprintf(stderr, "Tensor %s exists only in %s, not in %s\n", a->tensor->name, lhs->filename, rhs->filename);
            return false;
        }
        if (zero_tensor_equals(a->tensor, t, 1e-5, &max_diff) == false) {
            fprintf(stderr, "Tensor %s does NOT match\n", a->tensor->name);
            return false;
        }
        a = a->next;
    }
    while (b != NULL) {
        struct zero_tensor *t = zero_tensor_manager_get(lhs, b->tensor->name);
        if (t == NULL) {
            fprintf(stderr, "Tensor %s exists only in %s, not in %s\n", b->tensor->name, rhs->filename, lhs->filename);
            return false;
        }
        if (zero_tensor_equals(b->tensor, t, 1e-5, &max_diff) == false) {
            fprintf(stderr, "Tensor %s does NOT match\n", b->tensor->name);
            return false;
        }
        b = b->next;
    }
    return true;
}

void zero_tensor_manager_add(struct zero_tensor_manager *m, struct zero_tensor *t) {
    struct zero_tensor_allocation *a = (struct zero_tensor_allocation *)malloc(sizeof(struct zero_tensor_allocation));
    a->tensor = t;
    a->name_hash = zero_tensor_manager_fnv_hash(t->name);
    a->next = m->allocations;
    m->allocations = a;
    m->num_tensors++;
}

struct zero_tensor *zero_tensor_manager_get(struct zero_tensor_manager *m, char *name) {
    uint32_t name_hash = zero_tensor_manager_fnv_hash(name);
    struct zero_tensor_allocation *a = m->allocations;
    while (a != NULL) {
        if (a->name_hash == name_hash) {
            return a->tensor;
        }
        a = a->next;
    }
    return NULL;
}

void zero_tensor_manager_remove(struct zero_tensor_manager *m, char *name) {
    uint32_t name_hash = zero_tensor_manager_fnv_hash(name);
    struct zero_tensor_allocation *a = m->allocations;
    struct zero_tensor_allocation *prev = NULL;
    while (a != NULL) {
        if (a->name_hash == name_hash) {
            if (prev == NULL) {
                m->allocations = a->next;
            } else {
                prev->next = a->next;
            }
            m->num_tensors--;
            zero_tensor_free(a->tensor);
            free(a);
            return;
        }
        prev = a;
        a = a->next;
    }
}

void zero_tensor_manager_remove_all(struct zero_tensor_manager *m) {
    struct zero_tensor_allocation *a = m->allocations;
    while (a != NULL) {
        struct zero_tensor_allocation *next = a->next;
        zero_tensor_free(a->tensor);
        free(a);
        a = next;
    }
    m->allocations = NULL;
    m->num_tensors = 0;
}

int main() {
    struct zero_tensor_manager m;
    zero_tensor_manager_init(&m, "test.zero");
    struct zero_tensor xs[3];
    for (int i = 0; i < 3; i++) {
        char name[ZERO_MAX_TENSOR_NAME_LEN];
        sprintf(name, "x%d", i);
        int *shape = (int *)malloc(2 * sizeof(int));
        shape[0] = 2;
        shape[1] = 3;
        zero_tensor_init(&xs[i], name, ZERO_FLOAT32, 2, shape);
        zero_tensor_fill(&xs[i], i);
        zero_tensor_manager_add(&m, &xs[i]);
    }
    zero_tensor_manager_remove(&m, "x1");
    zero_tensor_manager_save(&m);
    struct zero_tensor_manager mm;
    zero_tensor_manager_init(&mm, "test.zero");
    zero_tensor_manager_load(&mm);
    bool ok = zero_tensor_manager_equals(&m, &mm);
    if (ok) {
        fprintf(stderr, "PASS\n");
    } else {
        fprintf(stderr, "FAIL\n");
    }
    return 0;
}


