#include <stdio.h>
#include <stdlib.h>

/**
Challenge 11 (Image segmentation). In addition to the C standard library, there
are many other support libraries out there that provide very different features.
Among those are a lot that do image processing of some kind.

Try to find a
suitable such image-processing library that is written in or interfaced to C and
that allows you to treat grayscale images as two-dimensional matrices of base
type unsigned char.

The goal of this challenge is to perform a segmentation of
such an image: to group the pixels (the unsigned char elements of the matrix)
into connected regions that are “similar” in some sense or another. Such a
segmentation forms a partition of the set of pixels, much as we saw in
challenge 4. Therefore, you should use a Union-Find structure to represent
regions, one per pixel at the start.

Can you implement a statistics function that computes a statistic for all
regions? This should be another array (the third array in the game) that for
each root holds the number of pixels and the sum of all values.

Can you implement a merge criterion for regions? Test whether the mean values of
two regions are not too far apart: say, no more than five gray values.

Can you implement a line-by-line merge strategy that, for each pixel on a line
of the image, tests whether its region should be merged to the left and/or to
the top? Can you iterate line by line until there are no more changes: that is,
such that the resulting regions/sets all test negatively with their respective
neighboring regions? Now that you have a complete function for image
segmentation, try it on images with assorted subjects and sizes, and also vary
your merge criterion with different values for the the mean distance instead of
five.
*/
// clang-22 chap8_chall11_image-segmentation.c $(pkg-config --cflags --libs
// MagickWand) -fopenmp=libgomp
int main(int argc, char *argv[]) {
  size_t n = 16;
  unsigned char img[16][16] = {
      {21, 22, 21, 21, 22, 22, 19, 19, 22, 21, 22, 19, 18, 21, 20, 19},
      {18, 22, 18, 22, 21, 21, 22, 19, 22, 18, 22, 18, 18, 18, 19, 19},
      {22, 18, 101, 100, 101, 102, 99, 102, 19, 20, 21, 18, 18, 21, 20, 21},
      {22, 18, 100, 100, 99, 102, 100, 98, 18, 22, 18, 21, 18, 20, 21, 18},
      {18, 18, 99, 99, 98, 101, 101, 101, 21, 18, 22, 19, 20, 20, 18, 20},
      {20, 18, 101, 98, 99, 99, 98, 98, 18, 21, 21, 19, 22, 19, 21, 22},
      {19, 19, 101, 101, 98, 101, 101, 99, 18, 20, 22, 20, 18, 19, 19, 21},
      {22, 22, 98, 98, 99, 99, 101, 100, 18, 22, 20, 20, 21, 18, 18, 18},
      {19, 22, 19, 18, 22, 20, 20, 22, 21, 19, 202, 201, 202, 19, 21, 19},
      {19, 20, 19, 22, 19, 19, 19, 22, 19, 201, 201, 202, 198, 201, 18, 18},
      {18, 18, 22, 20, 19, 21, 20, 21, 202, 201, 200, 202, 199, 198, 199, 19},
      {21, 22, 22, 22, 18, 20, 19, 19, 198, 198, 200, 201, 201, 199, 198, 18},
      {149, 150, 150, 152, 152, 149, 18, 20, 199, 201, 200, 202, 202, 199, 202,
       18},
      {148, 151, 150, 150, 148, 148, 22, 18, 21, 198, 200, 200, 199, 198, 18,
       21},
      {152, 150, 148, 149, 150, 150, 18, 21, 18, 21, 198, 201, 202, 18, 22, 21},
      {151, 152, 148, 152, 148, 148, 18, 18, 20, 21, 20, 21, 22, 21, 21, 21},
  };

  return EXIT_SUCCESS;
}