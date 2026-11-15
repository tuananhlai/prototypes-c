#include <assert.h>
#include <stdlib.h>

/**
Write a preprocessor condition that tests whether int has two’s complement sign
representation.
*/
int main(void) {
  static_assert(~0 == -1);
  return EXIT_SUCCESS;
}