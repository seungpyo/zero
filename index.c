#include "index.h"

hash_t zero_fnv_hash(char *str) {
    hash_t hash = 2166136261;
    while (*str) {
        hash = (hash ^ *str++) * 16777619;
    }
    return hash;
}

void zero_index_init(struct zero_index *index) {
    index->objects = NULL;
    index->num_objects = 0;
}

void zero_index_free(struct zero_index *index) {
    struct zero_index_object_list *cur, *next;
    for (cur = index->objects; cur != NULL; cur = next) {
        next = cur->next;
        free(cur->object);
        free(cur);
    }
}

void zero_index_add(struct zero_index *index, struct zero_index_object *object) {
    struct zero_index_object_list *l = (struct zero_index_object_list *)malloc(sizeof(struct zero_index_object_list));
    l->object = object;
    l->next = index->objects;
    index->objects = l;
    index->num_objects++;
}

void zero_index_remove(struct zero_index *index, hash_t hash) {
    for (struct zero_index_object_list *p = index->objects; p->next != NULL; p = p->next) {
        if (p->next->object->hash == hash) {
            struct zero_index_object_list *next = p->next->next;
            free(p->next->object);
            free(p->next);
            p->next = next;
            index->num_objects--;
            return;
        }
    }
}

struct zero_index_object *zero_index_get(struct zero_index *index, hash_t hash) {
    for (struct zero_index_object_list *p = index->objects; p != NULL; p = p->next) {
        if (p->object->hash == hash) {
            return p->object;
        }
    }
    return NULL;
}
