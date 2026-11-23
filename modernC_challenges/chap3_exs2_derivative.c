#include <stdio.h>

typedef double Function(double);

double deriv_real(Function f, double x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

double sample_fn(double x) { return x * x; }

int main() {
  double result = deriv_real(&sample_fn, 29);
  printf("%f\n", result);
  return 0;
}