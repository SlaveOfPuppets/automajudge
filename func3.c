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

#define NAME_SIZE 16
typedef struct {
  int a;
  int b;
  int c;
  char name[NAME_SIZE];
} test_structure_t;

#define MAX_CALLOCS 10

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break; It better not move.
  pbreak = sbrk(0);
  int counter = 0;

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

  // now on to the rest of the test.
  for (int i = 1; i < MAX_CALLOCS; i++) {
    test_structure_t *p = calloc(i, sizeof(test_structure_t));
    for (int entry = 0; entry < i; entry++) {
      p[entry].a = 1;
      p[entry].b = 2;
      p[entry].c = -3;
      sprintf(p[entry].name, "the entry is #%d\n", entry);
    }
    for (int entry = 0; entry < i; entry++) {
      counter++;
      myassertandscore(p[entry].a == 1, "memory corruption (a)", &score, 1);
      myassertandscore(p[entry].b == 2, "memory corruption (b)", &score, 1);
      myassertandscore(p[entry].c == -3, "memory corruption (c)", &score, 1);

      char buf[1024];
      sprintf(buf, "the entry is #%d\n", entry);
      int cmp = memcmp(p[entry].name, buf, strlen(buf));
      myassertandscore(cmp == 0, "memory corruption (on the end of the record)",
                       &score, 1);
    }
    free(p);
    p = NULL;
  }

  int realscore = 0;
  if (score == (counter * 4)) {
    realscore = 2;
  } else {
    if (score > (counter * 3))
      realscore = 1;
  }

  printf("%d\n", realscore);
  return 0;
}
