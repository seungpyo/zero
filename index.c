#include "index.h"
#include "tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

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
        // zero_index_object_free(cur->object);
        // free(cur);
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



int zero_index_save(struct zero_index *index, char *path) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        fprintf(stderr, "zero_index_save: failed to open file %s\n", path);
        fprintf(stderr, "zero_index_save: errno=%d\n", errno);
        return 1;
    }
    fwrite("ZERO", sizeof(char), 4, fp);
    fwrite(&index->num_objects, sizeof(uint32_t), 1, fp);

    uint64_t offset = ftell(fp) + sizeof(struct zero_index_object_ref) * index->num_objects;
    struct zero_index_object_ref *refs = (struct zero_index_object_ref *)malloc(sizeof(struct zero_index_object_ref) * index->num_objects);
    int i;
    struct zero_index_object_list *p;
    for (i = 0, p = index->objects; p != NULL; p = p->next, ++i) {
        refs[i] = (struct zero_index_object_ref){
            .hash = p->object->hash,
            .offset = offset,
            .size = p->object->size
        };
        offset += p->object->size;
        printf("zero_index_save: hash=%d offset=%llu size=%d type=%d\n", refs[i].hash, refs[i].offset, refs[i].size);
    }
    fwrite(refs, sizeof(struct zero_index_object_ref), index->num_objects, fp);

    struct zero_tensor * t; 
    for (i = 0, p = index->objects; p != NULL; p = p->next, ++i) {
        fseek(fp, refs[i].offset, SEEK_SET);
        switch (p->object->type) {
            case ZERO_BYTES:
                fwrite(p->object->data, sizeof(char), p->object->size, fp);
                break;
            case ZERO_TENSOR:
                t = (struct zero_tensor *)p->object->data;
                printf("zero_index_save: saving tensor %s\n", t->name);
                zero_tensor_print(t);
                zero_tensor_save(fp, (struct zero_tensor *)p->object->data);
                break;
            case ZERO_OPERATOR:
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
            case ZERO_BYTES:
                object->data = malloc(object->size);
                fread(object->data, sizeof(char), object->size, fp);
                break;
            case ZERO_TENSOR:
                zero_tensor_load(fp, (struct zero_tensor *)object->data);
                break;
            case ZERO_OPERATOR:
                fprintf(stderr, "zero_index_load: operator not implemented, ignoring...\n");
                break;
        }
        zero_index_add(index, object);
    }
    fclose(fp);
    return 0;
}

void zero_index_object_init(struct zero_index_object *object, hash_t hash, void *data, enum zero_index_object_type type) {
    object->hash = hash;
    object->type = type;
    object->data = data;
    object->size = zero_index_object_size(object);
}

void zero_index_object_free(struct zero_index_object *object) {
    free(object->data);
}

size_t zero_index_object_size(struct zero_index_object *object) {
    size_t header_size, data_size;
    switch (object->type) {
        case ZERO_BYTES:
            return strlen((char *)object->data) + 1;
        case ZERO_TENSOR:
            header_size = sizeof(struct zero_tensor) - sizeof(void *);
            data_size = zero_tensor_numel((struct zero_tensor *)object->data) * zero_dtype_size(((struct zero_tensor *)object->data)->dtype);
            printf("zero_index_object_size: header_size=%ld data_size=%ld\n", header_size, data_size);
            return header_size + data_size;
        case ZERO_OPERATOR:
            fprintf(stderr, "zero_index_object_size: operator not implemented\n");
            return 0;
    }
    return 0;
}