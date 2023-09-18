#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef uint32_t hash_t;

hash_t zero_fnv_hash(char *str) {
    hash_t hash = 2166136261;
    while (*str) {
        hash = (hash ^ *str++) * 16777619;
    }
    return hash;
}

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

int test_index() {
    time_t t0, t1;
    double elapsed;
    const int num_tests = 10000;
    const int max_string_length = 10000;
    char **test_strings = (char **)malloc(sizeof(char *) * num_tests);
    t0 = clock();
    for (int i = 0; i < num_tests; i++) {
        test_strings[i] = (char *)calloc(max_string_length, sizeof(char));
        for (int j = 0; j < max_string_length - 1; j++) {
            test_strings[i][j] = 'a' + (rand() % 26);
        }
        sprintf(test_strings[i], "test string %d", i);
    }
    t1 = clock();
    printf("Generated %d test strings in %f seconds\n", num_tests, (double)(t1 - t0) / CLOCKS_PER_SEC);

    struct zero_index index;
    zero_index_init(&index);
    printf("Initialized index\n");
    

    t0 = clock();
    for (int i = 0; i < num_tests; i++) {
        struct zero_index_object *object = (struct zero_index_object *)malloc(sizeof(struct zero_index_object));
        object->hash = zero_fnv_hash(test_strings[i]);
        object->data = test_strings[i];
        zero_index_add(&index, object);
    }
    t1 = clock();
    elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("ADD: num_tests=%d time=%fseconds average=%f\n", num_tests, elapsed, elapsed / num_tests);
    t0 = clock();
    for (int i = 0; i < num_tests; ++i) {
        struct zero_index_object *object = zero_index_get(&index, zero_fnv_hash(test_strings[i]));
        if (object == NULL) {
            printf("Object not found: %s\n", test_strings[i]);
            return 1;
        }
    }
    t1 = clock();
    elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("GET: num_tests=%d time=%fseconds average=%f\n", num_tests, elapsed, elapsed / num_tests);
    t0 = clock();
    for (int i = 0; i < num_tests; ++i) {
        zero_index_remove(&index, zero_fnv_hash(test_strings[i]));
    }
    t1 = clock();
    elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("REMOVE: num_tests=%d time=%fseconds average=%f\n", num_tests, elapsed, elapsed / num_tests);
    zero_index_free(&index);
    printf("Freed index\n");
    return 0;
}


int main() {
    test_index();
    return 0;
}

