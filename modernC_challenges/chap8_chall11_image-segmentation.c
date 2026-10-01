#include "chap4_exs1_unionfind.c"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"

#define MERGE_THRESHOLD 5

typedef struct {
  size_t count;
  size_t sum;
} RegionStats;

double region_mean(RegionStats stat) { return (double)stat.sum / stat.count; }

RegionStats region_merge(RegionStats dest, RegionStats src) {
  return (RegionStats){.count = dest.count + src.count,
                       .sum = dest.sum + src.sum};
}

/** Return the row major order of the given [i, j] coordinate.  */
size_t rm_index(size_t cols, size_t i, size_t j) { return i * cols + j; }

/**
 * Segment the given images into one or more regions based on pixel similarity
 * and write the results into `out_uf`.
 */
void segment(size_t h, size_t w, const unsigned char img[h][w],
             UnionFind *out_uf) {
  assert(out_uf->parent_len >= h * w);
  uf_reset(out_uf);

  RegionStats stats[h * w];
  for (size_t i = 0; i < sizeof(stats) / sizeof(RegionStats); i++) {
    stats[i].count = 1;
    stats[i].sum = img[i / w][i % w];
  }

  size_t cur_root, top_root, right_root;
  while (true) {
    bool has_merge = false;

    for (size_t i = 0; i < h; i++) {
      for (size_t j = 0; j < w; j++) {
        cur_root = uf_find(out_uf, rm_index(w, i, j));

        if (i > 0) {
          top_root = uf_find(out_uf, rm_index(w, i - 1, j));

          if (cur_root != top_root &&
              fabs(region_mean(stats[cur_root]) -
                   region_mean(stats[top_root])) <= MERGE_THRESHOLD) {
            has_merge = true;
            uf_union(out_uf, cur_root, top_root);
            cur_root = uf_find(out_uf, cur_root);
            stats[cur_root] = region_merge(stats[cur_root], stats[top_root]);
          }
        }

        if (j < w - 1) {
          right_root = uf_find(out_uf, rm_index(w, i, j + 1));

          if (cur_root != right_root &&
              fabs(region_mean(stats[cur_root]) -
                   region_mean(stats[right_root])) <= MERGE_THRESHOLD) {
            has_merge = true;
            uf_union(out_uf, cur_root, right_root);
            cur_root = uf_find(out_uf, cur_root);
            stats[cur_root] = region_merge(stats[cur_root], stats[right_root]);
          }
        }
      }
    }

    if (!has_merge)
      break;
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
and/or to the top?

Can you iterate line by line until there are no more changes: that is, such that
the resulting regions/sets all test negatively with their respective neighboring
regions? Now that you have a complete function for image segmentation, try it on
images with assorted subjects and sizes, and also vary your merge criterion with
different values for the the mean distance instead of five.
*/
// clang-22 chap8_chall11_image-segmentation.c $(pkg-config --cflags --libs
// MagickWand) -fopenmp=libgomp
