#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "index.h"
#include "tensor.h"

int test_index(int num_tests, int max_string_length) {
    time_t t0, t1;
    double elapsed;
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


int test_save_and_load(int num_test_strings, int num_test_tensors) {
    struct zero_index index;
    zero_index_init(&index);
    printf("Initialized index\n");
    char **test_strings = (char **)malloc(sizeof(char *) * num_test_strings);
    for (int i = 0; i < num_test_strings; ++i) {
        test_strings[i] = (char *)calloc(100, sizeof(char));
        snprintf(test_strings[i], 100, "test string %d", i);
        struct zero_index_object *object = (struct zero_index_object *)malloc(sizeof(struct zero_index_object));
        object->hash = zero_fnv_hash(test_strings[i]);
        object->data = test_strings[i];
        zero_index_add(&index, object);
    }
    printf("Added %d test strings to index\n", num_test_strings);
    struct zero_tensor *test_tensors = (struct zero_tensor *)malloc(sizeof(struct zero_tensor) * num_test_tensors);
    for (int i = 0; i < num_test_tensors; ++i) {
        char name[ZERO_MAX_TENSOR_NAME_LEN];
        sprintf(name, "test_tensor_%d", i);
        int *shape = (int *)malloc(2 * sizeof(int));
        shape[0] = 2;
        shape[1] = 3;
        zero_tensor_init(&test_tensors[i], name, ZERO_FLOAT32, 2, shape);
        zero_tensor_fill(&test_tensors[i], &i);
        struct zero_index_object *object = (struct zero_index_object *)malloc(sizeof(struct zero_index_object));
        object->hash = zero_fnv_hash(name);
        object->data = &test_tensors[i];
        zero_index_add(&index, object);
    }
    printf("Added %d test tensors to index\n", num_test_tensors);

    zero_index_save(&index, "test.zero");
    printf("Saved index to file\n");
    zero_index_free(&index);
    printf("Freed index\n");

    struct zero_index index2;
    zero_index_init(&index2);
    printf("Initialized index2\n");
    zero_index_load(&index2, "test.zero");
    printf("Loaded index from file\n");
    printf("Loaded index from file\n");
    for (int i = 0; i < num_test_strings; ++i) {
        struct zero_index_object *object = zero_index_get(&index2, zero_fnv_hash(test_strings[i]));
        if (object == NULL) {
            printf("Object not found: %s\n", test_strings[i]);
            return 1;
        }
    }
    printf("Verified %d test strings in index\n", num_test_strings);
    for (int i = 0; i < num_test_tensors; ++i) {
        char name[ZERO_MAX_TENSOR_NAME_LEN];
        sprintf(name, "test_tensor_%d", i);
        struct zero_index_object *object = zero_index_get(&index2, zero_fnv_hash(name));
        if (object == NULL) {
            printf("Object not found: %s\n", name);
            return 1;
        }
        float max_diff = 0;
        if (!zero_tensor_equals(&test_tensors[i], (struct zero_tensor *)object->data, 1e-4, &max_diff)) {
            printf("Tensor %s not equal\n", name);
            return 1;
        }
    }
    printf("Verified %d test tensors in index\n", num_test_tensors);
    zero_index_free(&index2);
    return 0;
}


int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s num_tests max_string_length\n", argv[0]);
        return 1;
    }

    int num_tests = atoi(argv[1]);
    int max_string_length = atoi(argv[2]);
    // test_index(num_tests, max_string_length);
    test_save_and_load(num_tests, num_tests);
    return 0;
}
