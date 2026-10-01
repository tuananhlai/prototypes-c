#define UNIT_TEST

#include "chap8_chall11_image-segmentation.c"

#include "../acutest.h"

void test_regions_uniform(void) {
  // Every pixel has the same value, so the whole image is one region.
  size_t h = 3, w = 3;
  unsigned char img[3][3] = {
      {50, 50, 50},
      {50, 50, 50},
      {50, 50, 50},
  };
  size_t n = h * w;
  UnionFind *uf = uf_create(n);

  segment(h, w, img, uf);

  size_t root = uf_find(uf, 0);
  for (size_t i = 1; i < n; i++) {
    TEST_CHECK_(uf_find(uf, i) == root, "expect pixel %zu in region %zu", i,
                root);
  }
  uf_destroy(uf);
}

void test_regions_three_regions(void) {
  // A 20 square, a 100 column and a 200 row.
  size_t h = 3, w = 3;
  unsigned char img[3][3] = {
      {20, 20, 100},
      {20, 20, 100},
      {200, 200, 200},
  };
  size_t n = h * w;
  UnionFind *uf = uf_create(n);

  segment(h, w, img, uf);

  TEST_CHECK_(uf_connected(uf, 0, 1) && uf_connected(uf, 1, 3) &&
                  uf_connected(uf, 3, 4),
              "expect 20 square (pixels 0, 1, 3, 4) to be one region");
  TEST_CHECK_(uf_connected(uf, 2, 5),
              "expect 100 column (pixels 2, 5) to be one region");
  TEST_CHECK_(uf_connected(uf, 6, 7) && uf_connected(uf, 7, 8),
              "expect 200 row (pixels 6, 7, 8) to be one region");
  TEST_CHECK_(!uf_connected(uf, 0, 2) && !uf_connected(uf, 2, 6),
              "expect 20, 100 and 200 to be separate regions");
  uf_destroy(uf);
}

void test_regions_threshold(void) {
  // Means 5 apart merge; 6 apart don't.
  unsigned char close[1][2] = {{10, 15}};
  unsigned char far[1][2] = {{10, 16}};
  UnionFind *uf = uf_create(2);

  segment(1, 2, close, uf);
  TEST_CHECK_(uf_connected(uf, 0, 1), "expect 10 and 15 merged");

  segment(1, 2, far, uf);
  TEST_CHECK_(!uf_connected(uf, 0, 1), "expect 10 and 16 not merged");
  uf_destroy(uf);
}

void test_backward_merge(void) {
  size_t h = 1, w = 3;
  unsigned char img[1][3] = {{1, 7, 4}};
  size_t n = h * w;
  UnionFind *uf = uf_create(n);

  segment(h, w, img, uf);

  TEST_CHECK_(uf_connected(uf, 0, 1) && uf_connected(uf, 1, 2),
              "expected pixel 0, 1, 2 to be connected");
  uf_destroy(uf);
}

TEST_LIST = {
    {"regions: uniform image is one region", test_regions_uniform},
    {"regions: three regions with stats", test_regions_three_regions},
    {"regions: merge threshold is 5", test_regions_threshold},
    {"regions: backward merge", test_backward_merge},
    {NULL, NULL},
};
