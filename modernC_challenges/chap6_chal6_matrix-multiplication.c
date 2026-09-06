#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t dot_product(const int32_t vec1[], const int32_t vec2[],
                    size_t vec_len) {
  int32_t retval = 0;
  for (size_t i = 0; i < vec_len; i++) {
    retval += vec1[i] * vec2[i];
  }
  return retval;
}

void matrix_vector_product(size_t m, size_t n, const int32_t (*matrix)[n],
                           const int32_t vec[], int32_t out_vec[]) {
  for (size_t i = 0; i < m; i++) {
    out_vec[i] = dot_product(matrix[i], vec, n);
  }
}

int main(void) {
  int32_t vec1[] = {1, 2, 3, 4};
  int32_t vec2[] = {9, 8, 7, 6};
  printf("%d\n", dot_product(vec1, vec2, sizeof(vec1) / sizeof(vec1[0])));

  int32_t matrix[3][4] = {
      {1, 2, 3, 4},
      {5, 6, 7, 8},
      {9, 10, 11, 12},
  };
  int32_t result_vec[3];
  matrix_vector_product(3, 4, matrix, vec1, result_vec);
  for (size_t i = 0; i < sizeof(result_vec) / sizeof(result_vec[0]); i++) {
    printf("%d\n", result_vec[i]);
  }
  return EXIT_SUCCESS;
}