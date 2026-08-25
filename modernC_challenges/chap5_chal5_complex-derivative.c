#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

typedef double complex (*Function)(double complex);

typedef struct {
  Function f;
} Derivative;

double complex evaluate(Derivative *de, double complex x) {
  double h = 1e-5;
  return (de->f(x + h) - de->f(x - h)) / (2.0 * h);
}

double complex square(double complex x) { return x * x; }

/// Can you extend the derivative (Challenge 2) to the complex domain: that is,
/// functions that receive and return double complex values?
int main(void) {
  Derivative derivative = {square};
  double complex res = evaluate(&derivative, I);
  printf("z = %.2f%+.2fi\n", creal(res), cimag(res));
  return EXIT_SUCCESS;
}