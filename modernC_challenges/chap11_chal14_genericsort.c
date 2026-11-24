#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*CompareFn)(void *a, void *b);

void ssort(void *base, size_t nmemb, size_t size, CompareFn compare_fn) {
  unsigned char *arr = base;
  for (size_t i = 0; i < nmemb; i++) {
    size_t min_idx = i;
    unsigned char min_val[size];
    memcpy(min_val, arr + min_idx * size, size);
    unsigned char j_val[size];
    for (size_t j = i + 1; j < nmemb; j++) {
      memcpy(min_val, arr + min_idx * size, size);
      memcpy(j_val, arr + j * size, size);
      if (compare_fn(j_val, min_val) == -1) {
        min_idx = j;
      }
    }

    memcpy(arr + min_idx * size, arr + i * size, size);
    memcpy(arr + i * size, min_val, size);
  }
}

int compare_double(void* a, void* b) {
  auto a_dbl = (double*) a;
  auto b_dbl = (double*) b;

  if (*a_dbl == *b_dbl) {
    return 0;
  } else if (*a_dbl < *b_dbl) {
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
  return EXIT_SUCCESS; 
}