#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

/// Can you extend the derivative (Challenge 2) to the complex domain: that is,
/// functions that receive and return double complex values?
int main(void) { 
  double complex a = 1 + I;
  double complex b = 1 - I;
  printf("%f\n", a * b);
  return EXIT_SUCCESS; 
}