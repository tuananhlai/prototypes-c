#include <stdio.h>
#include <stdlib.h>

#define sum(a, b) a + b
#define max(a, b) ((a) < (b) ? (b) : (a))

int main(void) {
  printf("sum(1, 2) = %d\n", sum(1, 2));
  printf("sum(2, 3) * 4 = %d\n", sum(2, 3) * 4); // should be 24

  printf("max(1, 2) = %d\n", max(1, 2));
  int a = 5;
  printf("before: a = 5, max(a--, 2) = %d\n", max(a--, 2));
  printf("after: a = %d\n", a);
  return EXIT_SUCCESS;
}