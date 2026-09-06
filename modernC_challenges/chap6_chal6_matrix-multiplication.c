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

void matrix_vector_product(size_t m, size_t n, const int32_t matrix[m][n],
                           const int32_t vec[], int32_t out_vec[]) {
  for (size_t i = 0; i < m; i++) {
    out_vec[i] = dot_product(matrix[i], vec, n);
  }
}

// TODO: find inversed matrix using Gaussian Elimination.
void matrix_inverse(size_t n, const int32_t matrix[n][n],
                    int32_t out_matrix[n][n]) {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      out_matrix[i][j] = -matrix[i][j];
    }
  }
}
