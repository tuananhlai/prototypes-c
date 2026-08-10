#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double calculate_pi(int num_digit);
double rand_double(double min, double max);

int main() {
  srand((unsigned int) time(NULL));
  double pi = calculate_pi(100000000);
  printf("%f\n", pi);
  return 0;
}

/// @brief calculate PI using Monte Carlo method. Image a 2x2 square circumscribed around a circle with r = 1. The area
/// of the square is 4, and the area of the inscribed circle is PI. We then randomly pick points within the square and
/// determine if that point also falls within the area of the circle as well.
/// The probability of this happening is PI / 4. So we say that PI ~ 4 * num_hit / num_attempts.
/// @param num_attempts number of random points to pick.
/// @return rough estimation of PI
double calculate_pi(int num_attempts) {
  int num_hit = 0;

  double x, y;
  for (int i = 0; i < num_attempts; i++) {
    x = rand_double(-1.0, 1.0);
    y = rand_double(-1.0, 1.0);

    if (x * x + y * y <= 1) {
      num_hit++;
    }
  }

  return 4.0 * num_hit / num_attempts;
}

double rand_double(double min, double max) {
  double v = (double)rand() / RAND_MAX;
  return (max - min) * v + min;
}