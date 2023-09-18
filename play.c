#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

int main() {
    int page_size = getpagesize();
    printf("page size: %d\n", page_size);
    struct rlimit rlim;
    getrlimit(RLIMIT_AS, &rlim);
    printf("rlim_cur: 0x%x\n", rlim.rlim_cur);
    printf("rlim_max: 0x%x\n", rlim.rlim_max);
    return 0;
}