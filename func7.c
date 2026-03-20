#define _GNU_SOURCE
#include "testhelper.h"
#include <pthread.h>
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
#define MAX_SIZE 40

#define NUM_ALLOCS 100
#define NUM_ITERATIONS 50000
#define NUM_THREADS 20

#define ALLOC 0
#define FREE 1

typedef struct {
  uint8_t *data;
  int size;
  int rnum;
  uint8_t backup[MAX_SIZE];
} alloc_t;

thread_local alloc_t allocs[NUM_ALLOCS];
thread_local int corruption_count = 0;
pthread_mutex_t scoring_lock = PTHREAD_MUTEX_INITIALIZER;
int __global__score = 0;

int minint(int x, int y) {
  if (x < y)
    return x;
  return y;
}

void *worker_thread_func(void *arg) {
  int *tid = (int *)arg;

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
        // allocs[ridx].size = *tid;
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

        // allocs[ridx].size = 64;
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
  for (int i = 0; i < NUM_ALLOCS; i++) {
    free(allocs[i].data);
  }

  pthread_mutex_lock(&scoring_lock);
  int realscore = 0;
  if (corruption_count == 0) {
    realscore = 1;
  } else {
    realscore = 0;
    fprintf(stderr, "Thread #%d corrupted %d blocks\n", *tid, corruption_count);
  }
  __global__score += realscore;
  pthread_mutex_unlock(&scoring_lock);

  return NULL;
}

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break; It better not move.
  pbreak = sbrk(0);
  srand(997);

  pthread_t threads[NUM_THREADS];
  int ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    ids[i] = 1 << i;
    pthread_create(&threads[i], NULL, worker_thread_func, &ids[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  if (__global__score < NUM_THREADS) {
    fprintf(stderr, "Allocator is not thread-safe! (%d threads had issues).\n",
            NUM_THREADS - __global__score);
    printf("0\n");
  } else {
    printf("1\n");
  }
  return 0;
}
