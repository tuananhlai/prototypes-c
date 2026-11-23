#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

/*
Challenge 13 (Generic derivative). Can you extend the real and complex
derivatives (challenges 2 and 5) such that they receive the function F and the
value x as a parameter?

Can you use the generic real derivatives to implement Newton’s method for
finding roots?

Can you find the real zeros of polynomials?

Can you find the complex zeros of polynomials?
*/

typedef double RealFn(double);

double deriv_real(RealFn f, double x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

typedef double complex ComplexFn(double complex);

double complex deriv_complex(ComplexFn f, double complex x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

#define deriv(F, X)                                                            \
  _Generic((X), double complex: deriv_complex, default: deriv_real)((F), (X))

double sample_fn(double x) { return x * x * x - 15 * x + 1000; }
double complex sample_complex_fn(double complex x) { return x * x; }

int main(void) {
  double v = deriv(sample_fn, 2.0);
  double complex v2 = deriv(sample_complex_fn, 1.0 + I);
  printf("%f %f + %fi\n", v, creal(v2), cimag(v2));
  return EXIT_SUCCESS;
}