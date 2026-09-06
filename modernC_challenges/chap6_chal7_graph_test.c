#include "chap6_chal7_graph.c"

#include "../acutest.h"
#define UNIT_TEST

void test_bfs(void) {
  bool adj_matrix[2][2] = {
      {true, true},
      {true, true},
  };
  size_t expected = 1;
  size_t actual = bfs(2, adj_matrix, 0, 1);
  TEST_CHECK_(actual == expected, "expect %zu, got %zu", expected, actual);
}

TEST_LIST = {
    {"check bfs", test_bfs},
    {NULL, NULL},
};