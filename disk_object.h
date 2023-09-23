#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "tensor.h"


// offset is determined by the separate allocator.
// e.g. tensor allocator, which allocates tensors in FCFS order.
// other members are determined by the serializer functions, i.e. when inflating the `data` buffer.
struct zero_disk_object {
    uint32_t hash;
    uint64_t offset;
    uint32_t size;
    void *data;
};

uint32_t zero_disk_object_hash(void *data, uint32_t size);

void zero_disk_object_write_header(struct zero_disk_object *obj, FILE *fp);
void zero_disk_object_read_header(struct zero_disk_object *obj, FILE *fp);
void zero_disk_object_write_data(struct zero_disk_object *obj, FILE *fp);
void zero_disk_object_read_data(struct zero_disk_object *obj, FILE *fp);

void zero_disk_object_serialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t);
void zero_disk_object_deserialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t);

void zero_disk_object_serialize_string(struct zero_disk_object *obj, char *str);
void zero_disk_object_deserialize_string(struct zero_disk_object *obj, char *str);
