#include "ops.h"
#include "tensor.h"


int main() {

    struct zero_tensor a, b, c, d;
    zero_tensor_init(&a, "a", ZERO_BFLOAT16, 2, (int[]){2, 3});
    zero_tensor_init(&b, "b", ZERO_BFLOAT16, 2, (int[]){3, 1});
    zero_tensor_init(&c, "c", ZERO_BFLOAT16, 2, (int[]){2, 1});
    zero_tensor_init(&d, "d", ZERO_BFLOAT16, 2, (int[]){2, 3});
    zero_ops_fill(&a, 1.0);
    zero_ops_fill(&b, 2.0);
    zero_ops_fill(&c, 3.0);
    zero_ops_fill(&d, 4.0);
    zero_tensor_print(&a);
    zero_tensor_print(&b);
    zero_tensor_print(&c);
    zero_tensor_print(&d);

    return 0;
}
