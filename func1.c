#define _GNU_SOURCE
#include "testhelper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

// Just initialize and exit
int main(int argc, char **argv) {
  // get the program break;
  pbreak = sbrk(0);

  int *iarr = malloc(sizeof(int));
  breakcheck();

  int *iarr2 = malloc(sizeof(int));
  breakcheck();

  myassert(samepagecheck(iarr, iarr2),
           "Two same-sized objects not allocated on same page, but should be.");
  myassertandscore(iarr != NULL && iarr2 != NULL, "malloc returned NULL!",
                   &score, 1);
  *iarr = TVAL;

  free(iarr);

  myassertandscore(TRUE, "", &score, 1);

  printf("%d\n", score);
  return 0;
}
