#include <assert.h>
#include <ctype.h>
#include <stdbit.h>
#include <stdio.h>
#include <stdlib.h>

double my_strtod(const char *nptr, char **endptr) {
  double sign = 1;
  double int_part = 0;
  double fractional_part = 0;

  size_t cur = 0;
  if (nptr[cur] == '-' || nptr[cur] == '+') {
    if (nptr[cur] == '-') {
      sign = -1;
    }
    cur++;
  }

  assert(isdigit(nptr[cur]));
  while (isdigit(nptr[cur])) {
    int_part = int_part * 10 + (nptr[cur] - '0');
    cur++;
  }

  if (nptr[cur] == '\0') {
    return sign * int_part;
  }
  assert(nptr[cur] == '.');
  cur++;
  assert(isdigit(nptr[cur]));

  size_t x = 1;
  while (isdigit(nptr[cur])) {
    fractional_part = fractional_part * 10 + (nptr[cur] - '0');
    x *= 10;
    cur++;
  }
  assert(nptr[cur] == '\0');

  return sign * (int_part + (double)fractional_part / x);
}