#define _GNU_SOURCE
#include "testhelper.h"
#include <stdbool.h>
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

#define NUM_SIZES 10
int malloc_sizes[NUM_SIZES] = {1, 3, 5, 8, 25, 456, 321, 102, 1002, 409};

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break;
  pbreak = sbrk(0);

  // quick check to make sure they are using something like a
  // segregated free-list
  void *p1, *p2, *p3, *p4, *p5, *p6;
  p1 = malloc(14);
  p2 = malloc(13);
  p3 = malloc(35);
  p4 = malloc(55);
  p5 = malloc(12);
  p6 = malloc(122);
  myassert(samepagecheck(p1, p2),
           "two 16-byte blocks should be on the same page, but aren't.");
  myassert(samepagecheck(p2, p5),
           "two 16-byte blocks should be on the same page, but aren't.");
  myassert(samepagecheck(p3, p4),
           "two 64-byte blocks should be on the same page, but aren't.");
  myassert(!samepagecheck(p1, p6),
           "blocks should be on different pages, but aren't.");
  myassert(!samepagecheck(p2, p6),
           "blocks should be on different pages, but aren't.");
  myassert(!samepagecheck(p3, p6),
           "blocks should be on different pages, but aren't.");
  myassert(!samepagecheck(p4, p6),
           "blocks should be on different pages, but aren't.");
  myassert(!samepagecheck(p5, p6),
           "blocks should be on different pages, but aren't.");

  void **pters = malloc(sizeof(void *) * NUM_SIZES);
  void **pbackup = malloc(sizeof(void *) * NUM_SIZES);

  for (int i = 0; i < NUM_SIZES; i++) {

    pters[i] = malloc(malloc_sizes[i]);
    breakcheck();
    myassertandscore(pters[i] != NULL, "malloc returned NULL!", &score, 1);

    pbackup[i] = malloc(malloc_sizes[i]);
    breakcheck();
    myassertandscore(pbackup[i] != NULL, "malloc returned NULL!", &score, 1);

    memset(pters[i], i, malloc_sizes[i]);
    memset(pbackup[i], i, malloc_sizes[i]);
  }

  for (int i = 0; i < NUM_SIZES; i++) {
    int cmp = memcmp(pters[i], pbackup[i], malloc_sizes[i]);
    myassertandscore(cmp == 0, "memory corruption occured.", &score, 1);

    free(pters[i]);
    free(pbackup[i]);
  }

  free(pters);
  free(pbackup);

  printf("%d\n", score);
  return 0;
}
