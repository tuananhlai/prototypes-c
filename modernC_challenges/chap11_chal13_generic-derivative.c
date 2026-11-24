#include <complex.h>
#include <math.h>
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

typedef double (*RealFn)(double);

double deriv_real(RealFn f, double x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

typedef double complex (*ComplexFn)(double complex);

double complex deriv_complex(ComplexFn f, double complex x) {
  double h = 1e-5;
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

#define deriv(F, X)                                                            \
  _Generic((X), double complex: deriv_complex, default: deriv_real)((F), (X))

double sample_fn(double x) { return x * x - 3; }
double complex sample_complex_fn(double complex x) { return x * x + x + 100; }

// Find the root (x so that f(x) == 0) using Newton's method.
bool find_root_real(RealFn f, double x0, double tol, size_t max_iter,
               double *out_root) {
  double guess_x = x0;
  size_t guess_attempts = 0;
  while (true) {
    if (guess_attempts == max_iter)
      break;

    double y = f(guess_x);
    if (fabs(y) < tol) {
      *out_root = guess_x;
      return true;
    };

    guess_x = guess_x - (y / deriv_real(f, guess_x));
    guess_attempts++;
  }

  return false;
}

bool find_root_complex(ComplexFn f, double complex x0, double tol, size_t max_iter,
               double complex *out_root) {
  double complex guess_x = x0;
  size_t guess_attempts = 0;
  while (true) {
    if (guess_attempts == max_iter)
      break;

    double complex y = f(guess_x);
    if (cabs(y) < tol) {
      *out_root = guess_x;
      return true;
    };

    guess_x = guess_x - (y / deriv_complex(f, guess_x));
    guess_attempts++;
  }

  return false;
}

int main(void) {
  double v = deriv(sample_fn, 2.0);
  double complex v2 = deriv(sample_complex_fn, 1.0 + I);
  printf("%f %f + %fi\n", v, creal(v2), cimag(v2));

  double complex root = 0;
  bool res = find_root_complex(sample_complex_fn, I, 1e-5, 10, &root);
  if (res) {
    printf("root found: %f %fi\n", creal(root), cimag(root));
  } else {
    puts("root not found");
  }
  return EXIT_SUCCESS;
}