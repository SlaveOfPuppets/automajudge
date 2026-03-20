#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

int64_t page_count = 0;

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
  void *(*orig_mmap)(void *, size_t, int, int, int, off_t);
  orig_mmap = dlsym(RTLD_NEXT, "mmap");
  void *result = orig_mmap(addr, length, prot, flags, fd, offset);
  if (result != MAP_FAILED) {
    page_count += (length + PAGE_SIZE - 1) >> 12;
  }
  return result;
}

int munmap(void *addr, size_t length) {
  int (*orig_munmap)(void *, size_t);
  orig_munmap = dlsym(RTLD_NEXT, "munmap");

  int result = orig_munmap(addr, length);
  if (result != 0) {
    page_count -= (length + PAGE_SIZE - 1) >> 12;
  }
  return result;
}

__attribute__((destructor)) static void libdone() {
  fprintf(stderr, "%ld\n", page_count);
}
