/******************************************************************************
 * libperf.h                                                                  *
 *                                                                            *
 * This file defines the libperf interface.  libperf is a library that wraps  *
 * around the syscall sys_perf_event_open( ).  This library exposes the       *
 * kernel performance counters subsystem to userspace code.  It can be used   *
 * to efficiently trace portions of code with kernel support.  Tracing        *
 * individual tasks/pids requires root access (as to some other functional    *
 * components).                                                               *
 *                                                                            *
 * This system call interface was first announced in 2009 by Ingo Molnar      *
 * (http://lkml.org/lkml/2009/6/6/149), and is now included in the Linux      *
 * kernel mainline.                                                           *
 *                                                                            *
 * Note: libperf is thread safe.  Multiple threads may use this library, but  *
 * each thread should call libperf_initialize separately and use their own    *
 * context returned from that call for subsequent library calls.              *
 *                                                                            *
 * Authors: Wolfgang Richter   <wolf@cs.cmu.edu>,                             *
 *          Ekaterina Taralova <etaralova@cs.cmu.edu>,                        *
 *          Karl Naden         <kbn@cs.cmu.edu>                               *
 *                                                                            *
 *                                                                            *
 * libperf interfaces with the kernel performance counters subsystem          *
 * Copyright (C) 2010  Wolfgang Richter, Ekaterina Taralova, Karl Naden       *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA              *
 * 02110-1301, USA.                                                           *
 ******************************************************************************/

#ifndef PERF_H
#define PERF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_STROPTS_H
#include <stropts.h>
#endif
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

enum perf_counters {
  /* sw tracepoints */
  COUNT_SW_CPU_CLOCK = 0,
  COUNT_SW_TASK_CLOCK = 1,
  COUNT_SW_CONTEXT_SWITCHES = 2,
  COUNT_SW_CPU_MIGRATIONS = 3,
  COUNT_SW_PAGE_FAULTS = 4,
  COUNT_SW_PAGE_FAULTS_MIN = 5,
  COUNT_SW_PAGE_FAULTS_MAJ = 6,

  /* hw counters */
  COUNT_HW_CPU_CYCLES = 7,
  COUNT_HW_INSTRUCTIONS = 8,
  COUNT_HW_CACHE_REFERENCES = 9,
  COUNT_HW_CACHE_MISSES = 10,
  COUNT_HW_BRANCH_INSTRUCTIONS = 11,
  COUNT_HW_BRANCH_MISSES = 12,
  COUNT_HW_BUS_CYCLES = 13,

  /* cache counters */

  /* L1D - data cache */
  COUNT_HW_CACHE_L1D_LOADS = 14,
  COUNT_HW_CACHE_L1D_LOADS_MISSES = 15,
  COUNT_HW_CACHE_L1D_STORES = 16,
  COUNT_HW_CACHE_L1D_STORES_MISSES = 17,
  COUNT_HW_CACHE_L1D_PREFETCHES = 18,

  /* L1I - instruction cache */
  COUNT_HW_CACHE_L1I_LOADS = 19,
  COUNT_HW_CACHE_L1I_LOADS_MISSES = 20,

  /* LL - last level cache */
  COUNT_HW_CACHE_LL_LOADS = 21,
  COUNT_HW_CACHE_LL_LOADS_MISSES = 22,
  COUNT_HW_CACHE_LL_STORES = 23,
  COUNT_HW_CACHE_LL_STORES_MISSES = 24,

  /* DTLB - data translation lookaside buffer */
  COUNT_HW_CACHE_DTLB_LOADS = 25,
  COUNT_HW_CACHE_DTLB_LOADS_MISSES = 26,
  COUNT_HW_CACHE_DTLB_STORES = 27,
  COUNT_HW_CACHE_DTLB_STORES_MISSES = 28,

  /* ITLB - instructiont translation lookaside buffer */
  COUNT_HW_CACHE_ITLB_LOADS = 29,
  COUNT_HW_CACHE_ITLB_LOADS_MISSES = 30,

  /* BPU - branch prediction unit */
  COUNT_HW_CACHE_BPU_LOADS = 31,
  COUNT_HW_CACHE_BPU_LOADS_MISSES = 32,

  /*LLC Events in KNL*/
  COUNT_HW_CACHE_LL_HIT_LOADS = 33,
  
  COUNT_PERF_EVENTS_MAX
};

#define __MAX_COUNTERS (COUNT_PERF_EVENTS_MAX)

static int perf_fds[__MAX_COUNTERS];

/* perf_event_open syscall wrapper */
static long sys_perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                                int cpu, int group_fd, unsigned long flags) {
  return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

/* gettid syscall wrapper */
inline pid_t gettid() { return syscall(SYS_gettid); }

/* perf specific */
static struct perf_event_attr perf_attrs[] = {

    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CPU_CLOCK},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_TASK_CLOCK},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CONTEXT_SWITCHES},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CPU_MIGRATIONS},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS_MIN},
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS_MAJ},

    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CPU_CYCLES},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_INSTRUCTIONS},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_REFERENCES},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_MISSES},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_MISSES},
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BUS_CYCLES},

    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config =
         (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) |
          (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    {.type = PERF_TYPE_HW_CACHE,
     .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},                
    /* { .type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_BPU |
       (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS <<
       16))}, */
};

static int enable_counter(int counter) {
  assert(counter >= 0 && counter < __MAX_COUNTERS);
  if (!perf_fds[counter]) {
    perf_attrs[counter].size = sizeof(struct perf_event_attr);
    perf_attrs[counter].inherit = 1;  /* default */
    perf_attrs[counter].disabled = 1; /* disable them now... */
    perf_attrs[counter].enable_on_exec = 0;

    int fd = sys_perf_event_open(&perf_attrs[counter], 0, -1, -1, 0);
    if (-1 == fd) {
      char error_text[100];
      snprintf(error_text, sizeof(error_text), "Cannot enable perf counter %d",
               counter);
      perror(error_text);

      return -1;
    }

    assert(fd);
    perf_fds[counter] = fd;
  }

  return ioctl(perf_fds[counter], PERF_EVENT_IOC_ENABLE);
}

static int disable_counter(int counter) {
  assert(counter >= 0 && counter < __MAX_COUNTERS);
  if (perf_fds[counter] == 0) return 0;

  return ioctl(perf_fds[counter], PERF_EVENT_IOC_DISABLE);
}

static uint64_t read_counter(int counter) {
  uint64_t value;
  assert(counter >= 0 && counter < __MAX_COUNTERS);
  if (perf_fds[counter] == 0) return 0;

  int bytes = read(perf_fds[counter], &value, sizeof(uint64_t));
  if (sizeof(uint64_t) != bytes) {
    perror("Failed reading performance events");
    return 0;
  }

  return value;
}

static inline __attribute__((always_inline))

void set_pmu(int pmu_index, uint8_t umask, uint8_t event) {
  assert(pmu_index >= 0 && pmu_index <= 3);
  write_msr(0x00410000ULL | umask << 8 | event, 0x186ULL + pmu_index, 8);
}

static inline __attribute__((always_inline))
uint64_t read_pmu(int pmu_index) {
  assert(pmu_index >= 0 && pmu_index <= 3);
  return read_msr(0xC1 + pmu_index, 8);
}

// The offcore response counters are specific to Knights Landing machines
#define UMASK_OFFCORE_RESP_0 (0x01)
#define EVENT_OFFCORE_RESP_0 (0xB7)
#define UMASK_OFFCORE_RESP_1 (0x01)
#define EVENT_OFFCORE_RESP_1 (0xBB)

static void set_offcore_resp_pmu(int pmu_index, int offcore_resp_index,
                                 uint32_t offcore_resp) {
  uint64_t msr_offset;
  if (offcore_resp_index == 0) {
    set_pmu(pmu_index, UMASK_OFFCORE_RESP_0, EVENT_OFFCORE_RESP_0);
    msr_offset = 0x1a6;
  } else {
    assert(offcore_resp_index == 1);
    set_pmu(pmu_index, UMASK_OFFCORE_RESP_1, EVENT_OFFCORE_RESP_1);
    msr_offset = 0x1a7;
  }

  write_msr(offcore_resp, msr_offset, 8);
}

#ifdef __cplusplus
}
#endif
#endif /* PERF_H */
