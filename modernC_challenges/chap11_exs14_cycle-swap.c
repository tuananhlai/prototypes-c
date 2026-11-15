#include <stdio.h>
#include <stdlib.h>

void cycle_swap(int* a, int* b, int* c) {
  int tmp = *a;
  *a = *b;
  *b = *c;
  *c = tmp;
}

int main(void) {
  int a = 1, b = 2, c = 3;
  cycle_swap(&a, &b, &c);
  printf("a=%d,b=%d,c=%d\n", a, b, c);
  return EXIT_SUCCESS;
}