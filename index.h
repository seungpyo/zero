#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint32_t hash_t;
hash_t zero_fnv_hash(char *str);

struct zero_index_object {
    hash_t hash;
    void *data;
};
struct zero_index_object_list {
    struct zero_index_object *object;
    struct zero_index_object_list *next;
};
struct zero_index {
    struct zero_index_object_list *objects;
    uint32_t num_objects;
};
void zero_index_init(struct zero_index *index);
void zero_index_free(struct zero_index *index);
void zero_index_add(struct zero_index *index, struct zero_index_object *object);
void zero_index_remove(struct zero_index *index, hash_t hash);
struct zero_index_object *zero_index_get(struct zero_index *index, hash_t hash);
