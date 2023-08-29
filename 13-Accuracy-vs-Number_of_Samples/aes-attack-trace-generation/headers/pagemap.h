#ifndef PAGEMAP_H
#define PAGEMAP_H

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
#include <sys/mman.h>

#include "common.h"

#define MAP_PHYS_MEM
#define RESERVED_PHYS_MEM_BEGIN    (0x1000000000)
#define RESERVED_PHYS_MEM_END      (0x1840000000)
#define MAX_PAGES  ((RESERVED_PHYS_MEM_END - RESERVED_PHYS_MEM_BEGIN) / PAGE_SIZE)

static uint64_t npages = 0;

#ifndef MAP_PHYS_MEM
#define MAX_PAGES         (1024 * 1024)

static FILE *pagemap_file;
static uint64_t actual_pa_bits = 0;

static uint64_t read_pagemap(volatile void *va)
{
    pagemap_file = fopen("/proc/self/pagemap", "rb");

    //Shifting by virt-addr-offset number of bytes
    //and multiplying by the size of an address (the size of an entry in pagemap file)
    int status = fseek(pagemap_file, (uint64_t) va / PAGE_SIZE * PAGEMAP_ENTRY, SEEK_SET);
    if (status) {
        perror("Failed to do fseek!");
        return -1;
    }

    uint64_t data = 0;
    unsigned char c_buf[PAGEMAP_ENTRY];
    for (int i = 0; i < PAGEMAP_ENTRY; i++) {
        int c = getc(pagemap_file);
        if (EOF == c) {
            perror("Reached end of the file\n");
            return 0;
        }
        c_buf[PAGEMAP_ENTRY - i - 1] = c;
    }

    for (int i = 0; i < PAGEMAP_ENTRY; i++)
        data = (data << 8) + c_buf[i];

    fclose(pagemap_file);

    uint64_t pa = GET_PFN(data) * PAGE_SIZE;
    actual_pa_bits |= pa;
    return pa;
}
#endif

#ifdef MAP_PHYS_MEM
static uint64_t system_ram_begin = 0, system_ram_end = 0;
static uint64_t system_ram_size = 0;
static void* system_ram_map_begin = NULL;
static void* system_ram_map_end = NULL;
static int mem_fd = -1;

static
void find_and_map_system_ram(void)
{
#ifndef RESERVED_PHYS_MEM_BEGIN
    FILE *f = fopen("/proc/iomem", "r");
    if (NULL == f) {
        perror("Failed to open /proc/iomem");
        exit(-1);
    }

    char line[80];
    while (NULL != fgets(line, 80, f)) {
        char* sep = strchr(line, ':');
        if (NULL == sep)
            continue;

        *sep = '\0';
        for (sep++ ; isspace(*sep) ; sep++);

        uint64_t begin, end, size;
        if (2 != sscanf(line, "%lx-%lx", &begin, &end))
            continue;

        assert(begin < end);
        end++;
        size = end - begin;

        if (!strncmp(sep, "System RAM", 10)) {
            if (size > system_ram_size) {
                system_ram_begin = begin;
                system_ram_end = end;
                system_ram_size = size;
            }
        } else {
            if (begin >= system_ram_begin && begin < system_ram_end) {
                system_ram_begin = end;
                system_ram_size = system_ram_end - system_ram_begin;
            }
        }
    }
#else
    system_ram_begin = RESERVED_PHYS_MEM_BEGIN;
    system_ram_end   = RESERVED_PHYS_MEM_END;
    system_ram_size  = system_ram_end - system_ram_begin;
#endif

    if (mem_fd == -1) {
        mem_fd = open("/dev/mem", O_RDWR);
        if (-1 == mem_fd) {
            perror("Failed to open /dev/mem");
            exit(-1);
        }
    }

    uint64_t try_map_size = 1;
    while (try_map_size < system_ram_end) try_map_size <<= 1;

    void* ptr = mmap(NULL, try_map_size * 2,
                     PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE,
                     -1, 0);

    if ((void*)-1 == ptr) {
        perror("Failed to find_aligned_address");
        exit(-1);
    }

    ptr = mmap((void*)(((uint64_t)ptr + try_map_size - 1) / try_map_size * try_map_size),
               system_ram_size,
               PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FILE|MAP_FIXED,
               mem_fd, system_ram_begin);

    if ((void*)-1 == ptr) {
        perror("Failed to map /dev/mem");
        exit(-1);
    }

    system_ram_map_begin = ptr;
    system_ram_map_end = (void*) ((uint64_t)ptr + system_ram_size);

    fprintf(stderr, "System RAM: %lx-%lx (size = %lu) mapped at %p-%p\n",
            system_ram_begin, system_ram_end, system_ram_size,
            system_ram_map_begin, system_ram_map_end);
}

#endif

static ADDR get_page(bool populate)
{
    if (npages >= MAX_PAGES) {
        //printf("exiting because of out-of-memory.\n");
        //exit(0);
        return NULL;
    }

#ifdef MAP_PHYS_MEM
    void *ptr = (void*)((volatile uintptr_t*) system_ram_map_begin + npages * PAGE_SIZE);
#else
    void *ptr = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE,
                     MAP_PRIVATE|MAP_ANON, -1, 0);

    if ((void*)-1 == ptr) {
        perror("Failed to allocate memory");
        return NULL;
    }
#endif

    ADDR target = (ADDR) ptr;

#ifndef MAP_PHYS_MEM
    if (populate) {
        uint64_t pa = 0;
        while (0 == pa) {
            target->val = 0;
            pa = read_pagemap(target);
        }
        mlock((void *) target, PAGE_SIZE);
        target->pa = pa;
    }
#else
    if (populate) {
        target->val = 0;
        target->pa = system_ram_begin + npages * PAGE_SIZE;
        target->next = NULL;
    }
#endif

    npages++;
    return target;
}

#ifdef __cplusplus
}
#endif

#endif // !PAGEMAP_H
