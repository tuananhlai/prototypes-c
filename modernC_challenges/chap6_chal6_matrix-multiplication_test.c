#include "../acutest.h"

#define UNIT_TEST
#include "chap6_chal6_matrix-multiplication.c"

void test_dot_product(void) {
  int32_t vec1[] = {1, 2, 3, 4};
  int32_t vec2[] = {9, 8, 7, 6};
  int32_t result = dot_product(vec1, vec2, 4);
  TEST_CHECK(result == 70);
  TEST_MSG("expected 70, got %d", result);

  result = dot_product(vec1, vec1, 0);
  TEST_CHECK(result == 0);
  TEST_MSG("expected 0, got %d", result);
}

void test_matrix_vector_product(void) {
  int32_t matrix[3][4] = {
      {1, 2, 3, 4},
      {5, 6, 7, 8},
      {9, 10, 11, 12},
  };
  int32_t vec[] = {1, 2, 3, 4};
  int32_t expected[3] = {30, 70, 110};
  int32_t out[3];
  matrix_vector_product(3, 4, matrix, vec, out);
  for (size_t i = 0; i < 3; i++) {
    TEST_CHECK_(out[i] == expected[i], "out[%zu] == %d (got %d)", i,
                expected[i], out[i]);
  }
}

void test_matrix_inverse(void) {
  int32_t matrix[2][2] = {
      {2, 1},
      {5, 3},
  };
  int32_t expected[2][2] = {
      {3, -1},
      {-5, 2},
  };
  int32_t inversed_matrix[2][2];
  matrix_inverse(2, matrix, inversed_matrix);
  for (size_t i = 0; i < 2; i++) {
    for (size_t j = 0; j < 2; j++) {
      TEST_CHECK_(inversed_matrix[i][j] == expected[i][j],
                  "inversed_matrix[%zu][%zu] == %d (got %d)", i, j,
                  expected[i][j], inversed_matrix[i][j]);
    }
  }
}

TEST_LIST = {
    {"dot_product", test_dot_product},
    {"matrix_vector_product", test_matrix_vector_product},
    {"matrix_inverse", test_matrix_inverse},
    {NULL, NULL},
};
