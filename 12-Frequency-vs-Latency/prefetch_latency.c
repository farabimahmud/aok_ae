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

typedef struct _param {
  void* addr_d0;
  int expt_idx;
} param;

void* thread0 (void* arg) {
  //printf("thread0 started\n");
  param* p = (param*)arg;

  int i;
  for (i = 0; i < 100000; i++) {
    pthread_mutex_lock(&lock);
    while (turn != 0)
    {
      pthread_cond_wait(&cond, &lock);
    }
    if (i % 2 == 0) {
      *((int*)p->addr_d0) ^= i;
    }
    asm volatile("mfence");
    turn = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  }
}

void* thread1 (void* arg) {
  //printf("thread1 started\n");
  param* p = (param*)arg;

  int i,j;
  for (i = 0; i < 100000; i++) {
    pthread_mutex_lock(&lock);
    while (turn != 1)
    {
      pthread_cond_wait(&cond, &lock);
    }
    asm volatile("mfence");
    int t1 = rdtscp();
    //__builtin_prefetch(p->addr_d0, 1, 3);  // rw: 1 (set to M state), 3: strong temporal locality
    asm __volatile__("prefetchw (%0)" : : "r"(p->addr_d0));
    asm volatile("mfence");
    int t2 = rdtscp();
    if (i % 2 == 0) {
      printf("w,%d\n", t2 - t1);
    } else {
      printf("n,%d\n", t2 - t1);
    }
    turn = 0;
    // interval
    for (j = 0; j < 1000; j++);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  }
}


int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("Usage: %s core1 core2\n", argv[0]);
    exit(1);
  }
  int th0_core = atoi(argv[1]);
  int th1_core = atoi(argv[2]);
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

  // Creat thread 0
  if (pthread_create(&th0, NULL, thread0, (void*)&p) != 0)
  {
    perror("thread0 cannot be created");
  }

  if (pthread_setaffinity_np(th0, sizeof(cpu_set_t), &cpuset) != 0)
  {
    perror("Cannot schedule thread0 on core 0");
  }
  
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

  pthread_join(th0, NULL);
  pthread_join(th1, NULL);

  close(fd);

  return 0;
}
