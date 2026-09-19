#include <stdio.h>
#include <stdlib.h>

struct tib {
  unsigned tib_sec : 6;
  unsigned tib_min : 6;
  unsigned tib_hour : 5;
};

struct tbi {
  unsigned _BitInt(6) tbi_sec : 6;
  unsigned _BitInt(6) tbi_min : 6;
  unsigned _BitInt(5) tbi_hour : 5;
};

int main(void) {
  struct tib t = {.tib_hour = 12, .tib_min = 27, .tib_sec = 60};
  struct tbi tb = {.tbi_hour = 23};
  printf("sizeof(t)=%zu, sizeof(tb)=%zu\n", sizeof(t), sizeof(tb));
  return EXIT_SUCCESS;
}