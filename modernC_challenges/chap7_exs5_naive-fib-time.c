#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t fib(size_t n) {
  if (n < 3)
    return 1;
  return fib(n - 1) + fib(n - 2);
}

size_t fib_cache_rec(size_t n, size_t cache[static n]) {
  if (!cache[n - 1]) {
    cache[n - 1] = fib_cache_rec(n - 1, cache) + fib_cache_rec(n - 2, cache);
  }
  return cache[n - 1];
}

size_t fib_cache(size_t n) {
  if (n < 3)
    return 1;
  size_t cache[n];
  memset(cache, 0, n * sizeof(size_t));
  cache[0] = 1;
  cache[1] = 1;
  return fib_cache_rec(n, cache);
}

void fib2rec(size_t n, size_t buf[static 2]) {
  if (n < 3) return;

  size_t retval = buf[0] + buf[1];
  buf[1] = buf[0];
  buf[0] = retval;
  fib2rec(n - 1, buf);
}

size_t fib2(size_t n) {
  size_t buf[2] = {1, 1};
  fib2rec(n, buf);
  return buf[0];
}

size_t fib_iterative(size_t n) {
  if (n < 3) return 1;
  size_t a1 = 1, a2 = 1;
  size_t tmp;
  for (size_t i = 0; i < n - 2; i++) {
    tmp = a1 + a2;
    a1 = a2;
    a2 = tmp;
  }

  return a2;
}

typedef size_t Fib(size_t n);

/**
Measure the times for calls to fib(n) with n set to different values. On POSIX
systems, you can use /bin/time to measure the run time of a program’s execution.

Example: time /a.out <n>
*/
int main(int argc, char *argv[]) {
  if (argc < 3) {
    puts("fib ( naive | cache | optimized-cache ) N");
    exit(EXIT_FAILURE);
  }
  int n = atoi(argv[2]);

  Fib *selected_fib;
  if (strcmp(argv[1], "cache") == 0) {
    selected_fib = fib_cache;
  } else if (strcmp(argv[1], "optimized-cache") == 0) {
    selected_fib = fib2;
  } else if (strcmp(argv[1], "iterative") == 0) {
    selected_fib = fib_iterative;
  } else {
    selected_fib = fib;
  }

  printf("%zu\n", selected_fib(n));
  return EXIT_SUCCESS;
}