#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <x86intrin.h>

#include <string>
#include <thread>

#include "argparse.hpp"

#define HUGEPAGE_SIZE 2 << 29
#define LLC_WAYS 16
#define L1_WAYS 8
#define L1SIZE (32 * 2 << 20)
#define CACHELINE_SIZE 64
#define WARMUP_COUNT 100
#define BITS 8

uint64_t THRESHOLD = 120;
uint64_t payload_size = 1;
uint16_t SECRET;
uint8_t* secrets = new uint8_t[payload_size];
char* canary = NULL;
bool mmap_allocated = false;

int fixed_source_cpuid = 0; 
int fixed_remote = 0; 
static bool flag = true;

typedef struct {
  uint64_t pfn : 55;
  unsigned int soft_dirty : 1;
  unsigned int file_page : 1;
  unsigned int swapped : 1;
  unsigned int present : 1;
} PagemapEntry;

/* Parse the pagemap entry for the given virtual address.
 *
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry* entry, int pagemap_fd, uintptr_t vaddr) {
  size_t nread;
  ssize_t ret;
  uint64_t data;
  uintptr_t vpn;

  vpn = vaddr / sysconf(_SC_PAGE_SIZE);
  nread = 0;
  while (nread < sizeof(data)) {
    ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                vpn * sizeof(data) + nread);
    nread += ret;
    if (ret <= 0) {
      return 1;
    }
  }
  entry->pfn = data & (((uint64_t)1 << 55) - 1);
  entry->soft_dirty = (data >> 55) & 1;
  entry->file_page = (data >> 61) & 1;
  entry->swapped = (data >> 62) & 1;
  entry->present = (data >> 63) & 1;
  return 0;
}

/* Convert the given virtual address to physical using /proc/PID/pagemap.
 *
 * @param[out] paddr physical address
 * @param[in]  pid   process to convert for
 * @param[in] vaddr virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int virt_to_phys_user(uintptr_t* paddr, uintptr_t vaddr) {
  char pagemap_file[] = "/proc/self/pagemap";
  int pagemap_fd;
  pagemap_fd = open(pagemap_file, O_RDONLY);
  if (pagemap_fd < 0) {
    return 1;
  }
  PagemapEntry entry;
  if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
    return 1;
  }
  close(pagemap_fd);
  *paddr =
      (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));
  return 0;
}

static inline void longnop() {
  asm volatile(
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
}

static inline unsigned long probe(char* adrs) {
  volatile unsigned long time;

  asm __volatile__(
      "    rdtsc              \n"
      "    lfence             \n"
      "    movl %%eax, %%esi  \n"
      "    movl (%1), %%eax   \n"
      "    lfence             \n"
      "    rdtsc              \n"
      "    subl %%esi, %%eax  \n"
      : "=a"(time)
      : "c"(adrs)
      : "%esi", "%edx");

  return time;
}

static inline unsigned long read_addr(char* adrs) {
  volatile unsigned long time;

  asm __volatile__("    movl (%1), %%eax   \n" : "=r"(time) : "c"(adrs));

  return time;
}

void* allocate_mmap() {
  canary = (char*)mmap((void*)0UL, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE,
                       (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB), 0, 0);
  if (canary == MAP_FAILED) {
    perror("Can't map memory");
    exit(1);
  }
  mmap_allocated = true;
  return canary;
}

void task0(char* addr, int fixed_src=4) {
  cpu_set_t cpuset0;
  CPU_ZERO(&cpuset0);
  CPU_SET(fixed_src, &cpuset0);
  int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset0);
  if (rc != 0) {
    std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
  }
  while (flag) {
    read_addr(addr);
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void task1(char* addr, int fixed_remote=66) {
  // for (int i = 0; i < 72; i += 2) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // if (i == fixed_source_cpuid) continue;
    cpu_set_t cpuset1;
    CPU_ZERO(&cpuset1);
    CPU_SET(fixed_remote, &cpuset1);
    int rc =
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset1);
    if (rc != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
    unsigned long remote_l2_hit_time = probe(addr);
    // virt_to_phys_user(&paddr, (uintptr_t)addr);
    printf("%p,%ld,%d\n", addr, remote_l2_hit_time, 
      fixed_remote);
    // printf("%p,%p,%ld,%d,%d\n", addr, paddr, remote_l2_hit_time, 
    // fixed_source_cpuid, fixed_remote);
  // }
  flag = false;
}

int main(int argc, const char** argv) {

  argparse::ArgumentParser parser("Profiling Cache Hit Time");
  parser.add_argument("-s", "--source-cpu")
      .help("Which CPU core we should pin one thread to, default 4")
      .default_value(0)
      .scan<'d', int>();
  parser.add_argument("-r", "--remote-cpu")
      .help("Which CPU core we should pin one thread to, default 4")
      .default_value(0)
      .scan<'d', int>();
  parser.add_argument("-of", "--offset-address")
      .help("Offset Address to add to the Buffer, default 62656")
      .default_value(62656)
      .scan<'d', int>();      
  try {
    parser.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << parser;
    std::exit(1);
  }

  fixed_source_cpuid = parser.get<int>("source-cpu");
  fixed_remote  = parser.get<int>("remote-cpu");
  uint64_t offset_addr = parser.get<int>("offset-address");
  char* buffer = (char*)allocate_mmap();

  // _mm_clflush(buffer);
  // unsigned long remote_l2_hit_time = probe(buffer);
  // printf("%p,%ld\n",buffer,remote_l2_hit_time);

  _mm_clflush(buffer+offset_addr);

  std::thread t0 = std::thread(task0, buffer+offset_addr, fixed_source_cpuid);
  std::thread t1 = std::thread(task1, buffer+offset_addr, fixed_remote);

  t0.join();
  t1.join();

  if (mmap_allocated) {
    munmap(buffer, HUGEPAGE_SIZE);
  }
  return 0;
}
