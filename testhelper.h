#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define TRUE 1
#define FALSE 0

#define myassert(COND, MSG)                                                    \
  ({                                                                           \
    do {                                                                       \
      if (!(COND)) {                                                           \
        fprintf(stderr,                                                        \
                "\tSTDERR-->\t%s --line " STR(__LINE__) " in " STR(            \
                    __FILE__) " \n",                                           \
                MSG);                                                          \
        fflush(stderr);                                                        \
      }                                                                        \
    } while (0);                                                               \
    COND;                                                                      \
  })

#define myassertandscore(COND, MSG, SVAR, S)                                   \
  ({                                                                           \
    do {                                                                       \
      if (myassert(COND, MSG)) {                                               \
        *SVAR += S;                                                            \
      }                                                                        \
    } while (0);                                                               \
    COND;                                                                      \
  })

#include <stdbool.h>
#include <stdint.h>
bool samepagecheck(void *p1, void *p2) {
  return (((uintptr_t)p1) >> 12) == (((uintptr_t)p2) >> 12);
}
