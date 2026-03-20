#define _GNU_SOURCE
#include "testhelper.h"
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
  pbreak = (void *)sbrk(0);

  int *iarr = malloc(0);
  breakcheck();
  free(iarr);

  printf("1\n");
  return 0;
}
