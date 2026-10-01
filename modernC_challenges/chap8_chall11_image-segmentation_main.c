#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  auto magickWand = NewMagickWand();
  // TODO: convert an image into an 2D array of unsigned char, put
  // it through `segment`, then construct a image which visualizes the
  // image regions.
  //
  // More specifically, we need to count how many regions are there and how many
  // pixels it have. Afterward, we need to assign a color to each pixels of these
  // regions. We can give them spaced-out values between 0 and 255. The more pixels a region have,
  // the lighter the pixel color.
  return EXIT_SUCCESS;
}