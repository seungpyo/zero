#include <stdio.h>
#include <stdint.h>
#include <math.h>

void show_hex(void *p) {
    uint8_t *ptr = (uint8_t *)p;
    for (int i = 0; i < 4; i++) {
        printf("%02x ", ptr[i]);
    }
    printf("\n");
}

float bfloat16_to_float(uint16_t b) {
    uint32_t tmp = ((uint32_t)b) << 16;
    return *((float *)&tmp);
}

uint16_t float_to_bfloat16(float x) {
    uint16_t b = (uint16_t)(*((uint32_t *)&x) >> 16);
    return b;
}


int main() {
    // float x = -1.5555;
    float x = 3.14f;
    printf("x = %f\n", x);
    uint16_t b = float_to_bfloat16(x);
    printf("b = %u\n", b);
    float y = bfloat16_to_float(b);
    printf("y = %f\n", y);
    float diff = fabs(x - y);
    printf("diff = %f\n", diff);
    show_hex(&x);
    show_hex(&y);

    if (diff < 1e-5) {
        printf("PASS, diff = %f\n", diff);
    } else {
        printf("FAIL, diff = %f\n", diff);
    }
    return 0;
}