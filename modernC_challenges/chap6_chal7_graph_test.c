#define UNIT_TEST

#include "chap6_chal7_graph.c"

#include "../acutest.h"

void test_bfs_small(void) {
  bool adj_matrix[2][2] = {
      {true, true},
      {true, true},
  };
  size_t expected = 1;
  size_t actual = bfs(2, adj_matrix, 0, 1);
  TEST_CHECK_(actual == expected, "expect dist = %zu, got %zu", expected,
              actual);
}

void test_bfs_small_nopath(void) {
  bool adj_matrix[2][2] = {
      {false, false},
      {false, false},
  };
  ssize_t expected = -1;
  ssize_t actual = bfs(2, adj_matrix, 0, 1);
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected,
              actual);
}

void test_bfs_small_same_start_end(void) {
  bool adj_matrix[2][2] = {
      {false, false},
      {false, false},
  };
  ssize_t expected = 0;
  ssize_t actual = bfs(2, adj_matrix, 0, 0);
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected,
              actual);
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
  TEST_CHECK_(actual == expected, "expect dist = %zd, got %zd", expected,
              actual);
}

void test_connected_components(void) {
  // Two components: the path 0-1-2 and the pair 3-4.
  const bool adj_matrix[5][5] = {
      {false, true, false, false, false}, {true, false, true, false, false},
      {false, true, false, false, false}, {false, false, false, false, true},
      {false, false, false, true, false},
  };
  size_t cc[5];
  connected_components(5, adj_matrix, cc);

  TEST_CHECK_(cc[0] == cc[1], "expect cc[0] == cc[1], got %zu vs %zu", cc[0],
              cc[1]);
  TEST_CHECK_(cc[1] == cc[2], "expect cc[1] == cc[2], got %zu vs %zu", cc[1],
              cc[2]);
  TEST_CHECK_(cc[3] == cc[4], "expect cc[3] == cc[4], got %zu vs %zu", cc[3],
              cc[4]);
  TEST_CHECK_(cc[0] != cc[3], "expect cc[0] != cc[3], both are %zu", cc[0]);
}

void test_spanning_tree_exists(void) {
  // Triangle 0-1-2: connected, so a spanning tree with exactly 2 edges exists.
  const bool adj_matrix[3][3] = {
      {false, true, true},
      {true, false, true},
      {true, true, false},
  };
  bool tree[3][3] = {0};
  int res = spanning_tree(3, adj_matrix, tree);
  TEST_CHECK_(res == 0, "expect spanning_tree = 0, got %d", res);

  size_t num_edges = 0;
  bool vertices[3] = {0};
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = i + 1; j < 3; j++) {
      TEST_CHECK_(tree[i][j] == tree[j][i], "expect symmetric edge %zu-%zu", i,
                  j);
      TEST_CHECK_(!tree[i][j] || adj_matrix[i][j],
                  "edge %zu-%zu not in original graph", i, j);
      if (tree[i][j]) {
        num_edges++;
        vertices[i] = true;
        vertices[j] = true;
      }
    }
  }
  TEST_CHECK_(num_edges == 2, "expect 2 edges, got %zu", num_edges);

  for (size_t i = 0; i < 3; i++) {
    TEST_CHECK_(vertices[i], "vertex %zu not covered", i);
  }
}

void test_spanning_tree_not_exists(void) {
  // Two components: 0-1 and isolated 2. No spanning tree covers all nodes.
  const bool adj_matrix[3][3] = {
      {false, true, false},
      {true, false, false},
      {false, false, false},
  };
  bool tree[3][3] = {0};
  int res = spanning_tree(3, adj_matrix, tree);
  TEST_CHECK_(res != 0, "expect spanning_tree != 0, got %d", res);
}

void test_queue(void) {
  Queue* q = queue_create();
  queue_enqueue(q, (Entry){.node = 0});
  queue_enqueue(q, (Entry){.node = 1});

  size_t expected_size = 2;
  size_t actual_size = queue_size(q);
  TEST_ASSERT_(actual_size == expected_size, "expect queue_size = %zu, got %zu",
               expected_size, actual_size);

  int res;
  Entry elem;
  res = queue_dequeue(q, &elem);
  TEST_ASSERT(res == 0);
  TEST_ASSERT(elem.node == 0);

  res = queue_dequeue(q, &elem);
  TEST_ASSERT(res == 0);
  TEST_ASSERT(elem.node == 1);

  res = queue_dequeue(q, &elem);
  TEST_ASSERT(res != 0);

  queue_destroy(q);
}

TEST_LIST = {
    {"check bfs", test_bfs_small},
    {"check bfs with the same start and end node",
     test_bfs_small_same_start_end},
    {"check bfs no path", test_bfs_small_nopath},
    {"check bfs big", test_bfs_big},
    {"check connected components", test_connected_components},
    {"check spanning tree exists", test_spanning_tree_exists},
    {"check spanning tree not exists", test_spanning_tree_not_exists},
    {"check queue", test_queue},
    {NULL, NULL},
};