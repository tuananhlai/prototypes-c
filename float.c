#include <stdio.h>
#include <stdlib.h>

int main(void) {
  double arr[5]; 
  for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
    printf("%f ", arr[i]);
  }
  putc('\n', stdout);

  double neg_zero = -0.0;
  double pos_zero = 0.0;
  printf("%d\n", pos_zero == neg_zero);
  return EXIT_SUCCESS;
}