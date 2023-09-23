#include "disk_object.h"

uint32_t zero_disk_object_hash(void *data, uint32_t size) {
    // fnv hash
    uint32_t hash = 2166136261;
    for (uint32_t i = 0; i < size; i++) {
        hash = (hash * 16777619) ^ ((uint8_t *)data)[i];
    }
    return hash;
}

int zero_disk_object_write_header_fd(struct zero_disk_object *obj, int fd, int offset) {
    FILE *fp = fdopen(fd, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[ZERO] fdopen failed\n");
        perror("[ZERO] fdopen");
        exit(1);
    }
    fseek(fp, offset, SEEK_SET);
    return  zero_disk_object_write_header(obj, fp);
}

int zero_disk_object_read_header_fd(struct zero_disk_object *obj, int fd, int offset) {
    FILE *fp = fdopen(fd, "rb");
    if (fp == NULL) {
        fprintf(stderr, "[ZERO] fdopen failed\n");
        perror("[ZERO] fdopen");
        exit(1);
    }
    fseek(fp, offset, SEEK_SET);
    return zero_disk_object_read_header(obj, fp);
}

int zero_disk_object_write_header(struct zero_disk_object *obj, FILE *fp) {
    fwrite(&(obj->hash), sizeof(uint32_t), 1, fp);
    fwrite(&(obj->offset), sizeof(uint64_t), 1, fp);
    fwrite(&(obj->size), sizeof(uint32_t), 1, fp);
    return ftell(fp);
}

int zero_disk_object_read_header(struct zero_disk_object *obj, FILE *fp) {
    fread(&(obj->hash), sizeof(uint32_t), 1, fp);
    fread(&(obj->offset), sizeof(uint64_t), 1, fp);
    fread(&(obj->size), sizeof(uint32_t), 1, fp);
    return ftell(fp);
}


int zero_disk_object_write_data_fd(struct zero_disk_object *obj, int fd, int offset) {
    FILE *fp = fdopen(fd, "wb");
    if (fp == NULL) {
        fprintf(stderr, "fdopen failed\n");
        perror("fdopen");
        exit(1);
    }
    fseek(fp, offset, SEEK_SET);
    return zero_disk_object_write_data(obj, fp);
}

int zero_disk_object_read_data_fd(struct zero_disk_object *obj, int fd, int offset) {
    FILE *fp = fdopen(fd, "rb");
    if (fp == NULL) {
        fprintf(stderr, "fdopen failed\n");
        perror("fdopen");
        exit(1);
    }
    fseek(fp, offset, SEEK_SET);
    return zero_disk_object_read_data(obj, fp);
}



int zero_disk_object_write_data(struct zero_disk_object *obj, FILE *fp) {
    fseek(fp, obj->offset, SEEK_SET);
    fwrite(obj->data, obj->size, 1, fp);
    return ftell(fp);
}

int zero_disk_object_read_data(struct zero_disk_object *obj, FILE *fp) {
    fseek(fp, obj->offset, SEEK_SET);
    fread(obj->data, obj->size, 1, fp);
    return ftell(fp);
}

void zero_disk_object_serialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t) {
    obj->hash = zero_disk_object_hash(t->name, strlen(t->name));
    uint32_t name_len = strlen(t->name) + 1;
    size_t buf_size = 0;
    buf_size += sizeof(uint32_t); // strlen(t->name)
    buf_size += name_len; // t->name
    buf_size += sizeof(uint32_t); // t->dtype
    buf_size += sizeof(uint32_t); // t->ndim
    buf_size += sizeof(uint32_t) * t->ndim; // t->shape
    buf_size += zero_tensor_numel(t) * zero_dtype_size(t->dtype); // t->data
    obj->data = malloc(buf_size);
    obj->size = buf_size;
    uint8_t *ptr = (uint8_t *)obj->data;
    memcpy(ptr, &name_len, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, t->name, name_len * sizeof(char));
    ptr += name_len;
    memcpy(ptr, &(t->dtype), sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, &(t->ndim), sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, t->shape, sizeof(uint32_t) * t->ndim);
    ptr += sizeof(uint32_t) * t->ndim;
    memcpy(ptr, t->data, zero_tensor_numel(t) * zero_dtype_size(t->dtype));
}
void zero_disk_object_deserialize_tensor(struct zero_disk_object *obj, struct zero_tensor *t) {
    uint8_t *ptr = (uint8_t *)obj->data;
    uint32_t name_len;
    char *name_buf;
    memcpy(&name_len, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    name_buf = malloc(name_len);
    memcpy(name_buf, ptr, name_len);
    strcpy(t->name, name_buf);
    free(name_buf);
    ptr += name_len;
    memcpy(&(t->dtype), ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(&(t->ndim), ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    t->shape = (int *)malloc(sizeof(uint32_t) * t->ndim);
    memcpy(t->shape, ptr, sizeof(uint32_t) * t->ndim);
    ptr += sizeof(uint32_t) * t->ndim;
    t->data = malloc(zero_tensor_numel(t) * zero_dtype_size(t->dtype));
    memcpy(t->data, ptr, zero_tensor_numel(t) * zero_dtype_size(t->dtype));
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