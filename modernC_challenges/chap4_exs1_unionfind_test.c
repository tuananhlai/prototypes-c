#include "chap4_exs1_unionfind.c"

#define UNIT_TEST
#include "../acutest.h"

void test_initial_state(void) {
  // Every element starts as its own singleton set.
  UnionFind *uf = uf_create(10);
  for (size_t i = 0; i < 10; i++) {
    size_t root = uf_find(uf, i);
    TEST_CHECK_(root == i, "expect find(%zu) = %zu, got %zu", i, i, root);
  }
  TEST_CHECK_(!uf_connected(uf, 0, 1), "expect 0 and 1 not connected");
  uf_destroy(uf);
}

void test_union_transitive(void) {
  // 0-1 and 1-2 joined, so 0 and 2 share a set; 3 stays apart.
  UnionFind *uf = uf_create(10);
  uf_union(uf, 0, 1);
  uf_union(uf, 1, 2);
  TEST_CHECK_(uf_connected(uf, 0, 2), "expect 0 and 2 connected");
  TEST_CHECK_(!uf_connected(uf, 0, 3), "expect 0 and 3 not connected");
  uf_destroy(uf);
}

void test_union_merges_groups(void) {
  // Two groups {0,1,2} and {3,4} merged through 2-4.
  UnionFind *uf = uf_create(10);
  uf_union(uf, 0, 1);
  uf_union(uf, 1, 2);
  uf_union(uf, 3, 4);
  TEST_CHECK_(!uf_connected(uf, 0, 3), "expect 0 and 3 not connected yet");

  uf_union(uf, 2, 4);
  for (size_t i = 0; i < 5; i++) {
    TEST_CHECK_(uf_connected(uf, 0, i), "expect 0 and %zu connected", i);
  }
  TEST_CHECK_(!uf_connected(uf, 0, 9), "expect 0 and 9 not connected");
  uf_destroy(uf);
}

void test_union_same_set(void) {
  // Joining two elements already in the same set changes nothing.
  UnionFind *uf = uf_create(4);
  uf_union(uf, 0, 1);
  uf_union(uf, 1, 0);
  uf_union(uf, 0, 0);
  TEST_CHECK_(uf_connected(uf, 0, 1), "expect 0 and 1 connected");
  TEST_CHECK_(!uf_connected(uf, 0, 2), "expect 0 and 2 not connected");
  uf_destroy(uf);
}

void test_find_replace(void) {
  // Chain 0 <- 1 <- 2 <- 3: every node on the path must point at the new root.
  UnionFind *uf = uf_create(5);
  uf->parent[1] = 0;
  uf->parent[2] = 1;
  uf->parent[3] = 2;

  uf_find_replace(uf, 3, 4);
  for (size_t i = 0; i < 4; i++) {
    TEST_CHECK_(uf->parent[i] == 4, "expect parent[%zu] = 4, got %zu", i,
                uf->parent[i]);
  }
  uf_destroy(uf);
}

void test_find_compress(void) {
  // Chain 0 <- 1 <- 2 <- 3: after compression every node points at root 0.
  UnionFind *uf = uf_create(4);
  uf->parent[1] = 0;
  uf->parent[2] = 1;
  uf->parent[3] = 2;

  size_t root = uf_find_compress(uf, 3);
  TEST_CHECK_(root == 0, "expect root = 0, got %zu", root);
  for (size_t i = 0; i < 4; i++) {
    TEST_CHECK_(uf->parent[i] == 0, "expect parent[%zu] = 0, got %zu", i,
                uf->parent[i]);
  }
  uf_destroy(uf);
}

TEST_LIST = {
    {"initial state: all singletons", test_initial_state},
    {"union: transitive", test_union_transitive},
    {"union: merges two groups", test_union_merges_groups},
    {"union: same set is a no-op", test_union_same_set},
    {"find_replace: rewrites whole path", test_find_replace},
    {"find_compress: flattens path", test_find_compress},
    {NULL, NULL},
};
