#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Show that if the minimal and maximal values for signed long long have
/// similar properties, the smallest integer value for the platform can’t be
/// written as a combination of one literal with a minus sign.
int main(void) {
  printf("%ld\n", -9223372036854775808);
  return EXIT_SUCCESS;
}