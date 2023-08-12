#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static inline uintptr_t align_up(uintptr_t sz, size_t alignment)
{
    uintptr_t mask = alignment - 1;
    if ((alignment & mask) == 0) {  /* power of two? */
        return ((sz + mask) & ~mask);       
    }
    return (((sz + mask) / alignment) * alignment);
}

int main(void)
{
    const int set = 4;
    const int elem = 4;
    const int sz = set * elem;
    unsigned long arr[sz];
    size_t alignment = 4;

    srand(time(0));

    for (int i = 0; i < set; ++i) {
        unsigned long res = rand() % 999;
        for (int j = 0; j < elem; ++j)
            arr[i * set + j] = res + j;
    }

    printf("Alignment = %lu\n", alignment);
    for (int i = 0; i < sz; ++i)
        printf("Test case %4d: before align_up = %4lu, after align_up = %4lu\n",
            i + 1, arr[i], align_up(arr[i], alignment));

    return 0;
}
