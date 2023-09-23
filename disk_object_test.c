#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk_object.h"

struct cat {
    char name[32];
    int age;
    int weight;
    bool meow;
};

struct dog {
    char name[32];
    int age;
    int weight;
    bool bark;
};

int main() {
    struct cat c = {"Kitty", 3, 5, true};
    struct dog d = {"Doggy", 5, 10, true};
    uint64_t offset = 0;
    struct zero_disk_object a = {
        .hash = 0,
        .type = 0,
        .offset = 100,
        .size = sizeof(struct cat)
    };
    struct zero_disk_object b = {
        .hash = 1,
        .type = 1,
        .offset = 100 + sizeof(struct cat),
        .size = sizeof(struct dog)
    };
    struct zero_disk_object so = {
        .hash = 2,
        .type = 2,
        .offset = 100 + sizeof(struct cat) + sizeof(struct dog),
        .size = strlen("hello")
    };
    FILE *fp = fopen("test.zero", "wb");
    zero_disk_object_write_header(&a, fp);
    zero_disk_object_write_header(&b, fp);
    zero_disk_object_write_header(&so, fp);
    zero_disk_object_write_data(&a, fp, &c);
    zero_disk_object_write_data(&b, fp, &d);
    zero_disk_object_write_data(&so, fp, "hello");
    fclose(fp);

    fp = fopen("test.zero", "rb");
    zero_disk_object_read_header(&a, fp);
    zero_disk_object_read_header(&b, fp);
    zero_disk_object_read_header(&so, fp);
    struct cat *c2 = malloc(a.size);
    struct dog *d2 = malloc(b.size);
    char *s2 = malloc(so.size);
    zero_disk_object_read_data(&a, fp, c2);
    zero_disk_object_read_data(&b, fp, d2);
    zero_disk_object_read_data(&so, fp, s2);
    if (c2->age == c.age && c2->weight == c.weight && c2->meow == c.meow) {
        printf("Cat data matches\n");
    } else {
        printf("Cat data does not match\n");
        printf("c1: %d %d %d\n", c.age, c.weight, c.meow);
        printf("c2: %d %d %d\n", c2->age, c2->weight, c2->meow);
    }
    if (d2->age == d.age && d2->weight == d.weight && d2->bark == d.bark) {
        printf("Dog data matches\n");
    } else {
        printf("Dog data does not match\n");
    }
    if (strcmp(s2, "hello") == 0) {
        printf("String data matches\n");
    } else {
        printf("String data does not match\n");
    }
    return 0;
}