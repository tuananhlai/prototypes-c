#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

typedef double complex Function(double complex);

double complex deriv_real(Function f, double complex x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

double complex sample_fn(double complex x) { return x * x; }

/// Can you extend the derivative (Challenge 2) to the complex domain: that is,
/// functions that receive and return double complex values?
int main(void) {
  double complex res = deriv_real(&sample_fn, I);
  printf("z = %.2f%+.2fi\n", creal(res), cimag(res));
  return EXIT_SUCCESS;
}