#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*CompareFn)(void const *a, void const *b);

// generic selection sort
void ssort(void *base, size_t nmemb, size_t size, CompareFn compare_fn) {
  unsigned char *arr = base;
  for (size_t i = 0; i < nmemb; i++) {
    size_t min_idx = i;
    for (size_t j = i + 1; j < nmemb; j++) {
      if (compare_fn(arr + j * size, arr + min_idx * size) < 0) {
        min_idx = j;
      }
    }

    if (min_idx != i) {
      for (size_t k = 0; k < size; k++) {
        unsigned char tmp = arr[min_idx * size + k];
        arr[min_idx * size + k] = arr[i * size + k];
        arr[i * size + k] = tmp;
      }
    }
  }
}

int compare_double(void const *a, void const *b) {
  auto a_dbl = (double *)a;
  auto b_dbl = (double *)b;

  // use comparison operator to avoid overflow issues.
  if (*a_dbl == *b_dbl) {
    return 0;
  } else if (*a_dbl < *b_dbl) {
    return -1;
  } else {
    return 1;
  }
}

int compare_sizet(void const *a, void const *b) {
  auto a_sizet = (size_t *)a;
  auto b_sizet = (size_t *)b;

  if (*a_sizet == *b_sizet) {
    return 0;
  } else if (*a_sizet < *b_sizet) {
    return -1;
  } else {
    return 1;
  }
}

int main(void) {
  double arr[] = {4, 10, 2, 1.5, 99};
  ssort(arr, sizeof(arr) / sizeof(arr[0]), sizeof(arr[0]), compare_double);
  for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
    printf("%f\n", arr[i]);
  }

  size_t arr2[] = {4, 10, 2, 105, 99};
  ssort(arr2, sizeof(arr2) / sizeof(arr2[0]), sizeof(arr2[0]), compare_double);
  for (size_t i = 0; i < sizeof(arr2) / sizeof(arr2[0]); i++) {
    printf("%zu\n", arr2[i]);
  }
  return EXIT_SUCCESS;
}