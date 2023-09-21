#include "index.h"
#include "tensor.h"

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

struct zero_index_object_ref {
    hash_t hash;
    uint64_t offset;
    uint32_t size;
};


int zero_index_save(struct zero_index *index, char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return 1;
    }
    fwrite(&index->num_objects, sizeof(uint32_t), 1, fp);
    for (struct zero_index_object_list *p = index->objects; p != NULL; p = p->next) {
        struct zero_index_object_ref ref;
        ref.hash = p->object->hash;
        ref.offset = ftell(fp);
        ref.size = p->object->size;
        fwrite(&ref, sizeof(struct zero_index_object_ref), 1, fp);
    }
    for (struct zero_index_object_list *p = index->objects; p != NULL; p = p->next) {
        switch (p->object->type) {
            case bytes:
                fwrite(p->object->data, sizeof(char), p->object->size, fp);
                break;
            case tensor:
                zero_tensor_save(fp, (struct zero_tensor *)p->object->data);
                break;
            case operator:
                fprintf(stderr, "zero_index_save: operator not implemented\n");
                fclose(fp);
                return 1;
                break;
        }
    }
    fclose(fp);
    return 0;
}

int zero_index_load(struct zero_index *index, char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return 1;
    }
    const char *magic = "ZERO";
    char buf[4];
    fread(buf, sizeof(char), 4, fp);
    if (strncmp(buf, magic, 4) != 0) {
        fprintf(stderr, "zero_index_load: invalid magic number\n");
        fclose(fp);
        return 1;
    }
    uint32_t num_objects;
    printf("Reading num_objects from offset %ld\n", ftell(fp));
    fread(&num_objects, sizeof(uint32_t), 1, fp);
    struct zero_index_object_ref *refs = (struct zero_index_object_ref *)malloc(sizeof(struct zero_index_object_ref) * num_objects);
    fread(refs, sizeof(struct zero_index_object_ref), num_objects, fp);
    printf("num_objects=%d\n", num_objects);
    for (int i = 0; i < num_objects; i++) {
        struct zero_index_object *object = (struct zero_index_object *)malloc(sizeof(struct zero_index_object));
        object->hash = refs[i].hash;
        object->size = refs[i].size;
        fseek(fp, refs[i].offset, SEEK_SET);
        switch (object->type) {
            case bytes:
                object->data = malloc(object->size);
                fread(object->data, sizeof(char), object->size, fp);
                break;
            case tensor:
                zero_tensor_load(fp, (struct zero_tensor *)object->data);
                break;
            case operator:
                fprintf(stderr, "zero_index_load: operator not implemented, ignoring...\n");
                break;
        }
        zero_index_add(index, object);
    }
    fclose(fp);
    return 0;
}
