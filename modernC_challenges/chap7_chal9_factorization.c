#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

size_t smallest_prime_factor(size_t n) {
  for (size_t i = 2; i <= sqrt(n); i++) {
    if (n % i == 0) {
      return i;
    }
  }

  return n;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    exit(EXIT_FAILURE);
  }
  size_t n;
  size_t factor;

  for (int i = 1; i < argc; i++) {
    n = atoi(argv[i]);
    printf("%zu:", n);
    while (n > 1) {
      factor = smallest_prime_factor(n);
      printf(" %zu", factor);
      while (n % factor == 0)
        n /= factor;
    }
    putchar('\n');
  }

  return EXIT_SUCCESS;
}