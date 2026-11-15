#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Show that the expressions -1U, -1UL and -1ULL have the maximum values
/// and type as the three non-promoted unsigned types, respectively.
int main(void) {
  printf("%u, %lu, %llu\n", -1U, -1UL, -1ULL);
  return EXIT_SUCCESS;
}