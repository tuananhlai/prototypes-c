#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/// @brief Show that if the maximum unsigned is 2^16 −1, 
/// then -0x8000 has value 32,768, too.
int main(void) {
  uint16_t v = -0x8000;
  printf("%d\n", v == 0x8000);
  return EXIT_SUCCESS;
}