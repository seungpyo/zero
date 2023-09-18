#include <stdio.h>
#include <time.h>
#include "index.h"

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

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s num_tests max_string_length\n", argv[0]);
        return 1;
    }
    int num_tests = atoi(argv[1]);
    int max_string_length = atoi(argv[2]);
    test_index(num_tests, max_string_length);
    return 0;
}
