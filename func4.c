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

#define MIN_SIZE 2
#define MAX_SIZE 4096

#define NUM_ITERATIONS 50

uint8_t memblock[MAX_SIZE];

int minint(int x, int y) {
  if (x < y)
    return x;
  return y;
}

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break; It better not move.
  pbreak = sbrk(0);
  srand(9);

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
  free(p1);
  free(p2);
  free(p3);
  free(p4);
  free(p5);
  free(p6);

  int lastsize = 2;
  uint8_t *pbuf = malloc(lastsize);

  for (int i = 0; i < MAX_SIZE; i++) {
    memblock[i] = rand() % 255;
  }
  memcpy(pbuf, memblock, lastsize);

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    int newsize = (rand() % (MAX_SIZE - MIN_SIZE)) + MIN_SIZE;

    int minsize = minint(newsize, lastsize);
    // printf("iter %d, %d -> %d (%d)\n",i,lastsize, newsize, minsize);

    pbuf = realloc(pbuf, newsize);

    int cmp = memcmp(pbuf, memblock, minsize);
    myassertandscore(cmp == 0, "memory not preserved by realloc.", &score, 1);

    memcpy(pbuf, memblock, newsize);
    lastsize = newsize;
  }

  free(pbuf);

  int realscore = 0;
  if (score == NUM_ITERATIONS) {
    realscore = 3;
  } else {
    if (score > (NUM_ITERATIONS / 2))
      realscore = 1;
  }

  printf("%d\n", realscore);
  return 0;
}
