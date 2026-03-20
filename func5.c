#define _GNU_SOURCE
#include "testhelper.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define MAX_SIZE 5000

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
        breakcheck();
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
        allocs[ridx].data = realloc(allocs[ridx].data, allocs[ridx].size);
        breakcheck();
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

  int realscore = 0;
  if (corruption_count == 0) {
    realscore = 5;
  } else if (corruption_count < 5) {
    realscore = 1;
    fprintf(stderr,
            "You got nearly everything right, but corrupted %d blocks\n",
            corruption_count);
  } else {
    fprintf(stderr, "You corrupted too many memory blocks. (%d)\n",
            corruption_count);
    abort();
  }

  printf("%d\n", realscore);
  return 0;
}
