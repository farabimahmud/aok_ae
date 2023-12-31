#define _GNU_SOURCE
#include <math.h>
#include <sched.h>
#include <pthread.h>
#include "util.h"

extern inline __attribute__((always_inline))
CYCLES rdtscp(void) {
	CYCLES cycles;
	asm volatile ("rdtscp"
	: /* outputs */ "=a" (cycles));

	return cycles;
}

/* 
 * Gets the value Time Stamp Counter 
 */
inline CYCLES get_time() {
    return rdtscp();
}

/* Synchronizes at the overflow of a counter
 */
extern inline __attribute__((always_inline))
CYCLES cc_sync() {
    while((get_time() % CHANNEL_SYNC_TIMEMASK) > CHANNEL_SYNC_JITTER) {
        }
    return get_time();
}



/*
 * Detects a bit by measuring the execution time of the prefetchw instruction 
 * within the clock length of config->interval.
 *
 * Detect a bit 1 if the exection time > PRE_MISS_LATENCY
 * Detect a bit 0 otherwise
 *
 * Within one iteration, the opereations from sender and receiver are ordered by the amount of rdtscp() each thread runs before the operation.
 * This is a weak ordering mechanism, and the amount of rdtscp() each threads needs (the value of i) may need to be adjusted on each processor
 */

void detect_bit(struct config *config, int index, int* result)
{
  int i = 0;
  CYCLES t1, t2;
  // Sync with sender
  CYCLES start_t = cc_sync();
  asm volatile("lfence");
  while (get_time() - start_t < config->interval) {
    if(i == 1)
    {
      t1 = rdtscp();
      //__builtin_prefetch(config->addr, 1, 3);
      asm __volatile__("prefetchw (%0)" : : "r"(config->addr));
      asm volatile("mfence");
      t2 = rdtscp();
      result[index] = ((t2 -t1) > PRE_HIT_LATENCY);
    }
    i++;
  }
}

void* receiver_func( void* param)
{
  // Initialize config and local variables
  struct config config;
  init_config(&config);
  int counter = 0;
  int result[ROUNDS];
  int a;

  //access config->addr so we can later prefetch
  CYCLES access_time = measure_one_block_access_time(config.addr);
  access_time = measure_one_block_access_time(config.addr);
  access_time = measure_one_block_access_time(config.addr);

  while (counter < ROUNDS) {
    asm volatile("lfence");
    detect_bit(&config, counter, result);
    counter ++;
  }

  for (a = 0; a < ROUNDS; a++)
    printf("%d\n", result[a]);

  printf("Receiver finished\n");
}


int main(int argc, char *argv[]) {
  pthread_t receiver;
  printf("%s\n", "Starting\n");

  pthread_attr_t attr_receiver;
  pthread_attr_init(&attr_receiver);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(receiver_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_receiver, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: receiver");
  }

  printf("created thread\n");

  if (pthread_create(&receiver, &attr_receiver, (void *)receiver_func, NULL) !=
      0) {
    perror("pthread_create: receiver");
  }

  pthread_join(receiver, NULL);
}



