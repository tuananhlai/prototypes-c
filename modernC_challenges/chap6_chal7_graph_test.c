#include "chap6_chal7_graph.c"

#include "../acutest.h"
#define UNIT_TEST

void test_bfs_small(void) {
  bool adj_matrix[2][2] = {
      {true, true},
      {true, true},
  };
  size_t expected = 1;
  size_t actual = bfs(2, adj_matrix, 0, 1);
  TEST_CHECK_(actual == expected, "expect dist = %zu, got %zu", expected, actual);
}

void test_bfs_small_nopath(void) {
  bool adj_matrix[2][2] = {
      {false, false},
      {false, false},
  };
  ssize_t expected = -1;
  ssize_t actual = bfs(2, adj_matrix, 0, 1);
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected, actual);
}

void test_bfs_small_same_start_end(void) {
  bool adj_matrix[2][2] = {
      {false, false},
      {false, false},
  };
  ssize_t expected = 0;
  ssize_t actual = bfs(2, adj_matrix, 0, 0);
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected, actual);
}

void test_bfs_big(void) {
  // Two routes from 0 to 4: 0-1-2-3-4 (length 4) and 0-5-4 (length 2).
  // BFS must return the shorter one.
  bool adj_matrix[6][6] = {
      {false, true, false, false, false, true},
      {true, false, true, false, false, false},
      {false, true, false, true, false, false},
      {false, false, true, false, true, false},
      {false, false, false, true, false, true},
      {true, false, false, false, true, false},
  };
  ssize_t expected = 2;
  ssize_t actual = bfs(6, adj_matrix, 0, 4);
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected, actual);
}

void test_queue(void) {
  Queue q = queue_create();
  queue_enqueue(&q, (Entry){.node = 0});
  queue_enqueue(&q, (Entry){.node = 1});

  size_t expected_size = 2;
  size_t actual_size = queue_size(&q);
  TEST_ASSERT_(actual_size == expected_size, "expect queue_size = %zu, got %zu",
               expected_size, actual_size);

  int res;
  Entry elem;
  res = queue_dequeue(&q, &elem);
  TEST_ASSERT(res == 0);
  TEST_ASSERT(elem.node == 0);

  res = queue_dequeue(&q, &elem);
  TEST_ASSERT(res == 0);
  TEST_ASSERT(elem.node == 1);

  res = queue_dequeue(&q, &elem);
  TEST_ASSERT(res != 0);

  queue_destroy(&q);
}

TEST_LIST = {
    {"check bfs", test_bfs_small},
    {"check bfs with the same start and end node", test_bfs_small_same_start_end},
    {"check bfs no path", test_bfs_small_nopath},
    {"check bfs big", test_bfs_big},
    {"check queue", test_queue},
    {NULL, NULL},
};