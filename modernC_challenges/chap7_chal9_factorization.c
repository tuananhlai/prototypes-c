#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"

void factorize(size_t n, size_t *factors) {
  arrpush(factors, 10);
  // for (size_t i = 2; n > 1; i++) {
  //   if (n % i != 0)
  //     continue;
  //   arrpush(factors, i);
  //   while (n % i == 0) n /= i;
  // }
}

int main(void) { 
  size_t n = 500;
  size_t* factors = NULL;
  factorize(n, factors);
  printf("%zu: ", n);
  for (size_t i = 0; i < arrlenu(factors); i++) {
    printf(" %zu", factors[i]);
  }
  arrfree(factors);
  return EXIT_SUCCESS; 
}