#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <x86intrin.h>

#define PAGE_SIZE         (4096)
#define CACHELINE_SIZE    (64)
#define PAGEMAP_ENTRY     (8)
#define GET_PFN(X)        ((X) & 0x7FFFFFFFFFFFFF)
#define CACHE_SET_OFFSET  (6)  // assuming 64 bytes / cacheline
#define PA_BITS           (39)

union addr {
    volatile int val;
    struct {
        volatile union addr *next;
        uint64_t pa;
    };
};

typedef volatile union addr *ADDR;

static inline __attribute__((always_inline))
uint64_t timed_access(volatile int *ptr)
{
    uint32_t hi1, lo1, hi2, lo2;
    int dummy;
    asm volatile("lfence\n\t"
                 "rdtsc\n\t"
                 "movl %%eax, %0\n\t"
                 "movl %%edx, %1\n\t"
                 "movl (%5), %2\n\t"
                 "lfence\n\t"
                 "rdtsc\n\t"
                 "movl %%eax, %3\n\t"
                 "movl %%edx, %4\n\t"
                 : "=r"(lo1), "=r"(hi1), "=r"(dummy), "=r"(lo2), "=r"(hi2)
                 : "r"(ptr)
                 : "eax", "edx", "memory");
    return ((uint64_t) hi2 << 32) + lo2 - ((uint64_t) hi1 << 32) - lo1;
}

static inline __attribute__((always_inline))
void noc_barrier(void)
{
    for (int i = 0; i < 100000; i++)
        asm volatile("nop");
}

static void shuffle_addrs(void** addrs, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        int j = rand() % (n - i) + i;
        void* temp = addrs[i];
        addrs[i] = addrs[j];
        addrs[j] = temp;
    }
}

#ifdef __cplusplus
}
#endif

#endif // !COMMON_H
