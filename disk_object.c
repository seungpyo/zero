#include "disk_object.h"

uint32_t zero_disk_object_hash(void *data, uint32_t size) {
    // fnv hash
    uint32_t hash = 2166136261;
    for (uint32_t i = 0; i < size; i++) {
        hash = (hash * 16777619) ^ ((uint8_t *)data)[i];
    }
    return hash;
}

void zero_disk_object_write_header(struct zero_disk_object *obj, FILE *fp) {
    fwrite(&(obj->hash), sizeof(uint32_t), 1, fp);
    fwrite(&(obj->offset), sizeof(uint64_t), 1, fp);
    fwrite(&(obj->size), sizeof(uint32_t), 1, fp);
}

void zero_disk_object_read_header(struct zero_disk_object *obj, FILE *fp) {
    fread(&(obj->hash), sizeof(uint32_t), 1, fp);
    fread(&(obj->offset), sizeof(uint64_t), 1, fp);
    fread(&(obj->size), sizeof(uint32_t), 1, fp);
}

void zero_disk_object_write_data(struct zero_disk_object *obj, FILE *fp) {
    fseek(fp, obj->offset, SEEK_SET);
    fwrite(obj->data, obj->size, 1, fp);
}

void zero_disk_object_read_data(struct zero_disk_object *obj, FILE *fp) {
    fseek(fp, obj->offset, SEEK_SET);
    fread(obj->data, obj->size, 1, fp);
}

void zero_disk_object_serialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t) {
    obj->hash = zero_disk_object_hash(t->name, strlen(t->name));
    size_t buf_size = 0;
    buf_size += ZERO_MAX_TENSOR_NAME_LEN; // t->name
    buf_size += sizeof(uint32_t); // t->dtype
    buf_size += sizeof(uint32_t); // t->ndim
    buf_size += sizeof(uint32_t) * t->ndim; // t->shape
    buf_size += zero_tensor_numel(t) * zero_dtype_size(t->dtype); // t->data
    obj->data = malloc(buf_size);
    obj->size = buf_size;
    uint8_t *buf = (uint8_t *)obj->data;
    uint8_t *buf_ptr = buf;
    memcpy(buf_ptr, t->name, ZERO_MAX_TENSOR_NAME_LEN);
    buf_ptr += ZERO_MAX_TENSOR_NAME_LEN;
    memcpy(buf_ptr, &(t->dtype), sizeof(uint32_t));
    buf_ptr += sizeof(uint32_t);
    memcpy(buf_ptr, &(t->ndim), sizeof(uint32_t));
    buf_ptr += sizeof(uint32_t);
    memcpy(buf_ptr, t->shape, sizeof(uint32_t) * t->ndim);
    buf_ptr += sizeof(uint32_t) * t->ndim;
    memcpy(buf_ptr, t->data, zero_tensor_numel(t) * zero_dtype_size(t->dtype));
}
void zero_disk_object_deserialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t) {
    uint8_t *buf = (uint8_t *)obj->data;
    uint8_t *buf_ptr = buf;
    memcpy(t->name, buf_ptr, ZERO_MAX_TENSOR_NAME_LEN);
    buf_ptr += ZERO_MAX_TENSOR_NAME_LEN;
    memcpy(&(t->dtype), buf_ptr, sizeof(uint32_t));
    buf_ptr += sizeof(uint32_t);
    memcpy(&(t->ndim), buf_ptr, sizeof(uint32_t));
    buf_ptr += sizeof(uint32_t);
    t->shape = (uint32_t *)malloc(sizeof(uint32_t) * t->ndim);
    memcpy(t->shape, buf_ptr, sizeof(uint32_t) * t->ndim);
    buf_ptr += sizeof(uint32_t) * t->ndim;
    t->data = malloc(zero_tensor_numel(t) * zero_dtype_size(t->dtype));
    memcpy(t->data, buf_ptr, zero_tensor_numel(t) * zero_dtype_size(t->dtype));
    return;
}

void zero_disk_object_serialize_string(struct zero_disk_object *obj, char *str) {
    obj->hash = zero_disk_object_hash(str, strlen(str));
    obj->data = (void *)str;
    obj->size = strlen(str);
    return;
}
void zero_disk_object_deserialize_string(struct zero_disk_object *obj, char *str) {
    memcpy(str, obj->data, obj->size);
    return;
}