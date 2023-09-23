#include "disk_object.h"

void zero_disk_object_write_header(struct zero_disk_object *obj, FILE *fp) {
    fwrite(obj, sizeof(struct zero_disk_object), 1, fp);
}

void zero_disk_object_read_header(struct zero_disk_object *obj, FILE *fp) {
    fread(obj, sizeof(struct zero_disk_object), 1, fp);
}

void zero_disk_object_write_data(struct zero_disk_object *obj, FILE *fp, void *data) {
    fseek(fp, obj->offset, SEEK_SET);
    fwrite(data, obj->size, 1, fp);
}

void zero_disk_object_read_data(struct zero_disk_object *obj, FILE *fp, void *data) {
    fseek(fp, obj->offset, SEEK_SET);
    fread(data, obj->size, 1, fp);
}
