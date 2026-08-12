#include <stdio.h>

typedef double (*Function)(double);

typedef struct {
  Function f;
} Derivative;

double calculate(Derivative* de, double x) {
  double h = 1e-5;
  return (de->f(x + h) - de->f(x - h)) / (2.0 * h);
}

double square(double x) { return x * x; }

int main() {
  Derivative derivative = {square};
  double result = calculate(&derivative, 29);
  printf("%f\n", result);
  return 0;
}