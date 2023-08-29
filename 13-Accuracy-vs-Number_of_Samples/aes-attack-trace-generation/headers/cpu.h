#ifndef CPU_H
#define CPU_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <sched.h>
#include <fcntl.h>

#include "common.h"

#define L1_CACHE_SET_MASK ((1ULL << 12) | (1ULL << 13) | (1ULL << 14) | (1ULL << 15) | \
                           (1ULL << 16) | (1ULL << 25) | (1ULL << 26) | (1ULL << 27) | \
                           (1ULL << 28) | (1ULL << 29))
#define L1_CACHE_SET(addr) (((uint64_t) addr) & L1_CACHE_SET_MASK)

static int num_of_levels = 0;
static int num_of_sets[4] = { -1, -1, -1, -1 };
static int num_of_ways[4] = { -1, -1, -1, -1 };

void set_cpu_parameters(void)
{
    char path[80];
    for (int i = 0 ;; i++) {
        snprintf(path, 80, "/sys/bus/cpu/devices/cpu0/cache/index%d/type", i);
        FILE *f = fopen(path, "r");
        if (!f) break;

        char buffer[16];
        if (fgets(buffer, 16, f) && strncmp(buffer, "Data", 4)) {
            fclose(f);

            snprintf(path, 80, "/sys/bus/cpu/devices/cpu0/cache/index%d/ways_of_associativity", i);
            f = fopen(path, "r");
            if (!f) break;
            fscanf(f, "%d", &num_of_ways[num_of_levels]);
            fclose(f);

            snprintf(path, 80, "/sys/bus/cpu/devices/cpu0/cache/index%d/number_of_sets", i);
            f = fopen(path, "r");
            if (!f) break;
            fscanf(f, "%d", &num_of_sets[num_of_levels]);

            num_of_levels++;
        }

        fclose(f);
    }

    if (num_of_levels == 0) {
        perror("Failed to get number of ways");
        exit(-1);
    }
}

static int current_cpu = -1;
static int msr_fd = -1;

static uint64_t read_msr(uint64_t offset, size_t size)
{
    if (-1 == current_cpu) {
        current_cpu = sched_getcpu();
        if (-1 == current_cpu) {
            perror("Failed to get current CPU\n");
            exit(-1);
        }
    }

    // if (-1 == msr_fd) {
    char path[80];
    snprintf(path, 80, "/dev/cpu/%d/msr", current_cpu);
    msr_fd = open(path, O_RDWR);
    if (-1 == msr_fd) {
        perror("Cannot change CPU MSR. Try run `sudo modprobe msr`.");
        exit(-1);
    }
    // }

    uint64_t val;
    pread(msr_fd, &val, size, offset);
    close(msr_fd);
    return val;
}

static void write_msr(uint64_t val, uint64_t offset, size_t size)
{
    if (-1 == current_cpu) {
        current_cpu = sched_getcpu();
        if (-1 == current_cpu) {
            perror("Failed to get current CPU\n");
            exit(-1);
        }
    }

    // if (-1 == msr_fd) {
    char path[80];
    snprintf(path, 80, "/dev/cpu/%d/msr", current_cpu);
    msr_fd = open(path, O_RDWR);
    if (-1 == msr_fd) {
        perror("Cannot change CPU MSR. Try run `sudo modprobe msr`.");
        exit(-1);
    }
    // }

    pwrite(msr_fd, &val, size, offset);
    close(msr_fd);
}

static void set_prefetcher(bool enable)
{
    uint64_t val = read_msr(0x1a4, 1);
    if (enable) val &= 0xfc; else val |= 0x03;
    write_msr(val, 0x1a4, 1);
    assert((read_msr(0x1a4, 1) & 0xf) == (val & 0xf));
}

static inline
uint64_t get_cache_set(ADDR addr, int level)
{
    if (level == 0)
#ifdef L1_CACHE_SET
        return L1_CACHE_SET(addr);
#else
        return ((uint64_t) addr >> CACHE_SET_OFFSET) % num_of_sets[0];
#endif
    else
#ifdef LLC_CACHE_SET
        return LLC_CACHE_SET(addr);
#else
        return (addr->pa >> CACHE_SET_OFFSET) % num_of_sets[level];
#endif
}

static bool switch_core(int core_id)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    bool cpu_set = false;
    char path[80];
    for (int i = 0 ;; i++) {
        int id;
        snprintf(path, 80, "/sys/bus/cpu/devices/cpu%d/topology/core_id", i);
        FILE* f = fopen(path, "r");
        if (NULL == f)
            break;
        fscanf(f, "%d", &id);
        fclose(f);
        if (id == core_id) {
            CPU_SET(id, &cpus);
            cpu_set = true;
        }
    }

    if (!cpu_set)
        return false;

    int ret = sched_setaffinity(0, sizeof(cpus), &cpus);
    if (ret == -1) {
        perror("Failed to set CPU affinity\n");
        return false;
    }

    sched_yield();
    sched_yield();
    sched_yield();

    int cpu = sched_getcpu();
    if (-1 == cpu) {
        perror("Failed to get current CPU\n");
        exit(-1);
    }

    if (!CPU_ISSET(cpu, &cpus))
         return false;

    current_cpu = cpu;
    if (-1 != msr_fd) {
        close(msr_fd);
        msr_fd = -1;
    }
    return true;
}

#ifdef __cplusplus
}
#endif

#endif // !CPU_H
