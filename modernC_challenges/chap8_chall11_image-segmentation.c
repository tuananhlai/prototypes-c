// Build: cc chap8_chall11_image-segmentation.c $(pkg-config --cflags --libs MagickWand)
// With clang, add -fopenmp=libgomp (clang's default OpenMP runtime, libomp, is not installed)
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

/**
Challenge 11 (Image segmentation). In addition to the C standard library, there
are many other support libraries out there that provide very different features.
Among those are a lot that do image processing of some kind. Try to find a
suitable such image-processing library that is written in or interfaced to C and
that allows you to treat grayscale images as two-dimensional matrices of base
type unsigned char. The goal of this challenge is to perform a segmentation of
such an image: to group the pix- els (the unsigned char elements of the matrix)
into connected regions that are “similar” in some sense or another. Such a
segmentation forms a partition of the set of pixels, much as we saw in
challenge 4. Therefore, you should use a Union-Find structure to represent
regions, one per pixel at the start.
Can you implement a statistics function that computes a statistic for all
regions? This should be another array (the third array in the game) that for
each root holds the number of pixels and the sum of all values. Can you
implement a merge criterion for regions? Test whether the mean values of two
regions are not too far apart: say, no more than five gray values.
Can you implement a line-by-line merge strategy that, for each pixel on a line
of the image, tests whether its region should be merged to the left and/or to
the top? Can you iterate line by line until there are no more changes: that is,
such that the resulting regions/sets all test negatively with their respective
neighboring regions? Now that you have a complete function for image
segmentation, try it on images with assorted subjects and sizes, and also vary
your merge criterion with different values for the the mean distance instead of
five.
*/
// clang-22 chap8_chall11_image-segmentation.c $(pkg-config --cflags --libs MagickWand) -fopenmp=libgomp
int main(int argc, char *argv[]) {
  if (argc < 2) return fprintf(stderr, "usage: %s image\n", argv[0]), EXIT_FAILURE;

  MagickWandGenesis();
  MagickWand *wand = NewMagickWand();
  if (MagickReadImage(wand, argv[1]) == MagickFalse)
    return fprintf(stderr, "cannot read %s\n", argv[1]), EXIT_FAILURE;

  size_t w = MagickGetImageWidth(wand);
  size_t h = MagickGetImageHeight(wand);

  unsigned char(*px)[w] = malloc(h * sizeof *px); // the 2D array
  MagickExportImagePixels(wand, 0, 0, w, h, "I", CharPixel, px);

  printf("%zux%zu, pixel[0][0] = %u\n", w, h, px[0][0]);

  free(px);
  DestroyMagickWand(wand);
  MagickWandTerminus();
  return EXIT_SUCCESS;
}