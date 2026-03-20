#define _GNU_SOURCE
#include "testhelper.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#define TVAL 123456

void *pbreak;
int score = 0;

void breakcheck() {
  void *newbreak = sbrk(0);
  if (!myassert(pbreak == newbreak,
                "program break should never change, but it does!")) {
    exit(-1);
  }
}

#define MIN_SIZE 0
#define MAX_SIZE 1025

#define NUM_ALLOCS 100
#define NUM_ITERATIONS 500000

#define ALLOC 0
#define FREE 1

typedef struct {
  uint8_t *data;
  int size;
  int rnum;
  uint8_t backup[MAX_SIZE];
} alloc_t;

alloc_t allocs[NUM_ALLOCS];
int corruption_count = 0;

int minint(int x, int y) {
  if (x < y)
    return x;
  return y;
}

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break; It better not move.
  pbreak = sbrk(0);
  srand(997);

  uint64_t start = clock();

  // initialize the array of allocations
  for (int i = 0; i < NUM_ALLOCS; i++) {
    allocs[i].data = NULL;
  }

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    // pick a random allocation
    int ridx = rand() % NUM_ALLOCS;
    int dice = rand() % 2; // pick a random action

    if (dice == ALLOC) {
      if (allocs[ridx].data == NULL) {
        allocs[ridx].size = rand() % MAX_SIZE;
        // allocs[ridx].size = 1024;
        allocs[ridx].rnum = rand() % 128;

        memset(allocs[ridx].backup, allocs[ridx].rnum, MAX_SIZE);
        allocs[ridx].data = malloc(allocs[ridx].size);
        // breakcheck();
        memset(allocs[ridx].data, allocs[ridx].rnum, allocs[ridx].size);
      } else {
        // check validity
        int cmp =
            memcmp(allocs[ridx].backup, allocs[ridx].data, allocs[ridx].size);
        if (cmp != 0)
          corruption_count++;

        allocs[ridx].size = rand() % MAX_SIZE;
        allocs[ridx].rnum = rand() % 128;

        memset(allocs[ridx].backup, allocs[ridx].rnum, MAX_SIZE);
        // allocs[ridx].data = realloc(allocs[ridx].data, allocs[ridx].size);
        free(allocs[ridx].data);
        allocs[ridx].data = malloc(allocs[ridx].size);
        // breakcheck();
        memset(allocs[ridx].data, allocs[ridx].rnum, allocs[ridx].size);
      }
    } else {
      // free it
      if (allocs[ridx].data != NULL) {
        int cmp =
            memcmp(allocs[ridx].backup, allocs[ridx].data, allocs[ridx].size);
        if (cmp != 0)
          corruption_count++;

        free(allocs[ridx].data);
        allocs[ridx].data = NULL;
      }
    }
  }

  uint64_t elapsed = clock() - start;
  double felapsed = ((double)elapsed) / CLOCKS_PER_SEC;

  struct tms tbuf;
  clock_t celapsed = times(&tbuf);

  int realscore = 0;
  if (corruption_count == 0) {
    realscore = 5;
    printf("%lld\n", tbuf.tms_stime);
  } else {
    fprintf(stderr, "You corrupted %d memory blocks.\n", corruption_count);
    abort();
  }
  return 0;
}
