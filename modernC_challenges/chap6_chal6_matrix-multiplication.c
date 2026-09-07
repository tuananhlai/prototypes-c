#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void swap_arr(double arr1[], double arr2[], size_t n) {
  double tmp;
  for (size_t i = 0; i < n; i++) {
    tmp = arr1[i];
    arr1[i] = arr2[i];
    arr2[i] = tmp;
  }
}

/**
 * Find the solution n x 1 vector `x` in `Ax = b` equation, where A is a 2D
 * matrix and b is the desired n x 1 vector.
 */
int gaussian_elimination(size_t n, const double a[n][n], const double b[n],
                         double out_x[n]) {
  double m[n][n + 1];
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n + 1; j++) {
      if (j < n) {
        m[i][j] = a[i][j];
      } else {
        m[i][j] = b[i];
      }
    }
  }

  size_t max_row;
  for (size_t k = 0; k < n - 1; k++) {
    max_row = k;
    for (size_t i = k + 1; i < n; i++) {
      if (fabs(m[i][k]) > fabs(m[max_row][k])) {
        max_row = i;
      }
    }

    // why swap and check for zero at that particular position?
    swap_arr(m[k], m[max_row], n + 1);
    if (m[k][k] == 0) {
      return -1;
    }

    double factor;
    for (size_t i = k + 1; i < n; i++) {
      factor = m[i][k] / m[k][k];
      for (size_t j = k; j <= n; j++) {
        m[i][j] = m[i][j] - factor * m[k][j];
      }
    }
  }

  double sum;
  for (ssize_t i = n - 1; i >= 0; i--) {
    sum = 0;
    for (size_t j = i + 1; j < n; j++) {
      sum += (m[i][j] * out_x[j]);
    }
    out_x[i] = (m[i][n] - sum) / m[i][i];
  }
  return 0;
}

int inverse_matrix(size_t n, const double matrix[n][n],
                   double out_matrix[n][n]) {
  double x[n];
  double b[n];
  for (size_t col = 0; col < n; col++) {
    for (size_t j = 0; j < n; j++) {
      b[j] = 0;
    }
    b[col] = 1;

    if (gaussian_elimination(n, matrix, b, x) != 0) {
      return -1;
    }

    for (size_t row = 0; row < n; row++) {
      out_matrix[row][col] = x[row];
    }
  }
  return 0;
}
