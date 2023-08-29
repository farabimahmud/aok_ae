#define _GNU_SOURCE
  #include <sys/mman.h>
#include <pthread.h>
#include <stdio.h>
#include <x86intrin.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "utils.h"
#include <fcntl.h>
#include <stdlib.h>


/*
* The __builtin_prefetch() function prefetches memory from addr. 
* The rationale is to minimize cache-miss latency by
* trying to move data into a cache before accessing the data.
* Possible use cases include frequently called sections of code
* in which it is known that the data in a given address is likely to be accessed soon.
* 
* In addition to addr, there are two optional stdarg(3) arguments, rw and locality.
* The value of the latter should be a compile-time constant integer between 0 and 3.
* The higher the value, the higher the temporal locality in the data.
* When locality is 0, it is assumed that there is little or no temporal locality in the data; 
* after access, it is not necessary to leave the data in the cache. 
* The default value is 3. The value of rw is either 0 or 1,
* corresponding with read and write prefetch, respectively.
* The default value of rw is 0. Also rw must be a compile-time constant integer.
* 
* The __builtin_prefetch() function translates into prefetch instructions
* only if the architecture has support for these.
* If there is no support, addr is evaluated only if it includes side effects,
* although no warnings are issued by gcc(1).
*/

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int turn = 0; // 0: thread0 1: thread1
uint32_t threshold;
typedef struct _param {
  void* addr_d0;
  int expt_idx;
} param;

typedef struct _tinfo{
  uint32_t t1, t2, latency;
} tinfo;

void* thread0 (void* arg) {
  printf("start spy\n");
  param* p = (param*)arg;
  srand(time(NULL));
  uint32_t timestamps[4];
  int junk = 123;
  int i;
  for (i = 0; i < 4; i++) {
    int r = rand() % 5;
    sleep(r);
    *((int*)p->addr_d0) ^= i;
    timestamps[i] = rdtscp();
  }

  for (i = 0; i < 4; i++) {
    printf("victim %d,%u\n", i, timestamps[i]);
  }
}

void* thread1 (void* arg) {
  printf("thread1 started\n");
  param* p = (param*)arg;
  tinfo timestamps[4];
  int i;

  threshold = 0;
  // warmming up
  for (i = 0; i < 2000; i++) {
    asm volatile("mfence");
      uint32_t t1 = rdtscp();
    asm __volatile__("prefetchw (%0)" : : "r"(p->addr_d0));
    asm volatile("mfence");
    uint32_t t2 = rdtscp();
    printf("spy warming up %d,%u\n", i, t2 - t1);
    threshold += (t2 - t1);
  }
  threshold = threshold / 2000;

  for (i = 0; i < 2000; i++) {
    asm volatile("mfence");
    uint32_t t1 = rdtscp();
    asm __volatile__("prefetchw (%0)" : : "r"(p->addr_d0));
    asm volatile("mfence");
  }

  printf("start detection with threshold %u\n", threshold);
  for (i = 0; i < 4; i++) {
    while (1) {
      asm volatile("mfence");
      uint32_t t1 = rdtscp();
      //__builtin_prefetch(param, 1, 3);
      asm __volatile__("prefetchw (%0)" : : "r"(p->addr_d0));
      asm volatile("mfence");
      uint32_t t2 = rdtscp();
      uint32_t latency = t2 - t1;

      if (latency > threshold) {
        timestamps[i].t1 = t1;
        timestamps[i].t2 = t2;
        timestamps[i].latency = latency;
        break;
      }
    }
  }

  for (i = 0; i < 4; i++) {
    printf("spy %d,%u,%u,%u\n", i,
        timestamps[i].t1,
        timestamps[i].t2,
        timestamps[i].latency);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s core1 core2 threshold\n", argv[0]);
    exit(1);
  }
  int th0_core = atoi(argv[1]);
  int th1_core = atoi(argv[2]);
  threshold = atoi(argv[3]);
 
  pthread_t th0, th1;
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(th0_core, &cpuset);

  param p;
  int fd = open("./shared", O_RDWR);
  if (fd < 0)
  {
    perror("file creation failed");
  }
  p.addr_d0 = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  p.expt_idx = 0;

  // Creat thread 1
  CPU_ZERO(&cpuset);
  CPU_SET(th1_core, &cpuset);
  // Creat thread 0
  if (pthread_create(&th1, NULL, thread1, (void*)&p) != 0)
  {
    perror("thread1 cannot be created");
  }

  if (pthread_setaffinity_np(th1, sizeof(cpu_set_t), &cpuset) != 0)
  {
    perror("Cannot schedule thread0 on core 1");
  }
 
  sleep(5);
  printf("start victim\n");
  // Creat thread 0
  if (pthread_create(&th0, NULL, thread0, (void*)&p) != 0)
  {
    perror("thread0 cannot be created");
  }

  if (pthread_setaffinity_np(th0, sizeof(cpu_set_t), &cpuset) != 0)
  {
    perror("Cannot schedule thread0 on core 0");
  }
  

  pthread_join(th0, NULL);
  pthread_join(th1, NULL);

  close(fd);

  return 0;
}
