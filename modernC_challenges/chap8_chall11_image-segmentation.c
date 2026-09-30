#include "chap4_exs1_unionfind.c"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"

typedef struct {
  size_t count;
  size_t sum;
} RegionStats;

double region_mean(RegionStats stat) { return (double)stat.sum / stat.count; }

RegionStats region_merge(RegionStats dest, RegionStats src) {
  return (RegionStats){.count = dest.count + src.count,
                       .sum = dest.sum + src.sum};
}

size_t to_1d_index(size_t w, size_t i, size_t j) { return i * w + j; }

void regions(size_t h, size_t w, const unsigned char img[h][w],
             UnionFind *out_uf, RegionStats *out_stats) {
  assert(out_uf->parent_len >= h * w);
  uf_reset(out_uf);

  for (size_t i = 0; i < h; i++) {
    for (size_t j = 0; j < w; j++) {
      size_t idx = to_1d_index(w, i, j);
      out_stats[idx].count = 1;
      out_stats[idx].sum = img[i][j];
    }
  }

  RegionStats top_stat, cur_stat, right_stat;
  size_t cur_idx, top_idx, right_idx;
  for (size_t i = 0; i < h; i++) {
    for (size_t j = 0; j < w; j++) {
      cur_idx = uf_find(out_uf, to_1d_index(w, i, j));
      cur_stat = out_stats[cur_idx];

      if (i > 0) {
        top_idx = uf_find(out_uf, to_1d_index(w, i - 1, j));
        top_stat = out_stats[top_idx];

        if (!uf_connected(out_uf, cur_idx, top_idx) &&
            fabs(region_mean(cur_stat) - region_mean(top_stat)) <= 5) {
          uf_union(out_uf, cur_idx, top_idx);
          // TODO: refactor to improve readability
          cur_idx = uf_find(out_uf, cur_idx);
          out_stats[cur_idx] = region_merge(cur_stat, top_stat);
          cur_stat = out_stats[cur_idx];
        }
      }

      if (j < w - 1) {
        right_idx = uf_find(out_uf, to_1d_index(w, i, j + 1));
        right_stat = out_stats[right_idx];

        if (!uf_connected(out_uf, cur_idx, right_idx) &&
            fabs(region_mean(cur_stat) - region_mean(right_stat)) <= 5) {
          uf_union(out_uf, cur_idx, right_idx);
          cur_idx = uf_find(out_uf, cur_idx);
          out_stats[cur_idx] = region_merge(cur_stat, right_stat);
        }
      }
    }
  }
}

/**
Challenge 11 (Image segmentation). In addition to the C standard library,
there are many other support libraries out there that provide very different
features. Among those are a lot that do image processing of some kind.

Try to find a
suitable such image-processing library that is written in or interfaced to C
and that allows you to treat grayscale images as two-dimensional matrices of
base type unsigned char.

The goal of this challenge is to perform a segmentation of
such an image: to group the pixels (the unsigned char elements of the
matrix) into connected regions that are “similar” in some sense or another.
Such a segmentation forms a partition of the set of pixels, much as we saw
in challenge 4. Therefore, you should use a Union-Find structure to
represent regions, one per pixel at the start.

Can you implement a statistics function that computes a statistic for all
regions? This should be another array (the third array in the game) that for
each root holds the number of pixels and the sum of all values.

Can you implement a merge criterion for regions? Test whether the mean
values of two regions are not too far apart: say, no more than five gray
values.

Can you implement a line-by-line merge strategy that, for each pixel on a
line of the image, tests whether its region should be merged to the left
and/or to the top? Can you iterate line by line until there are no more
changes: that is, such that the resulting regions/sets all test negatively
with their respective neighboring regions? Now that you have a complete
function for image segmentation, try it on images with assorted subjects and
sizes, and also vary your merge criterion with different values for the the
mean distance instead of five.
*/
// clang-22 chap8_chall11_image-segmentation.c $(pkg-config --cflags --libs
// MagickWand) -fopenmp=libgomp
#ifndef UNIT_TEST
int main(void) {
  size_t h = 5, w = 5;
  unsigned char img[5][5] = {
      {20, 20, 20, 20, 20},   
      {20, 100, 100, 20, 20}, 
      {20, 100, 100, 20, 20},
      {20, 20, 20, 200, 200}, 
      {20, 20, 20, 200, 200},
  };

  size_t n = h * w;
  RegionStats stats[n];
  auto uf = uf_create(n);

  regions(h, w, img, uf, stats);
  for (size_t i = 0; i < n; i++) {
    size_t root = uf_find(uf, i);
    printf("i = %zu, root = %zu, count = %zu, sum = %zu\n", i, root,
           stats[root].count, stats[root].sum);
  }

  uf_destroy(uf);
  return EXIT_SUCCESS;
}
#endif // UNIT_TEST