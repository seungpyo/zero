#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct zero_disk_object {
    uint32_t hash;
    uint32_t type;
    uint64_t offset;
    uint32_t size;
};

void zero_disk_object_write_header(struct zero_disk_object *obj, FILE *fp);
void zero_disk_object_read_header(struct zero_disk_object *obj, FILE *fp);
void zero_disk_object_write_data(struct zero_disk_object *obj, FILE *fp, void *data);
void zero_disk_object_read_data(struct zero_disk_object *obj, FILE *fp, void *data);