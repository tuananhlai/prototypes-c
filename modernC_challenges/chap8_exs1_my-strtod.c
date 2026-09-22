#include <assert.h>
#include <ctype.h>
#include <stdbit.h>
#include <stdio.h>
#include <stdlib.h>

double my_strtod(const char *nptr, const char **endptr) {
  double sign = 1;
  double int_part = 0;
  double fractional_part = 0;
  double retval;
  size_t consumed = 0;

  size_t cur = 0;
  while (isspace(nptr[cur]))
    cur++;

  if (nptr[cur] == '-' || nptr[cur] == '+') {
    if (nptr[cur] == '-') {
      sign = -1;
    }
    cur++;
  }

  if (!isdigit(nptr[cur])) {
    retval = 0;
    goto cleanup;
  }

  while (isdigit(nptr[cur])) {
    int_part = int_part * 10 + (nptr[cur] - '0');
    cur++;
    consumed = cur;
  }

  if (nptr[cur] != '.') {
    retval = sign * int_part;
    goto cleanup;
  }
  cur++;

  if (!isdigit(nptr[cur])) {
    retval = sign * int_part;
    goto cleanup;
  }

  size_t x = 1;
  while (isdigit(nptr[cur])) {
    fractional_part = fractional_part * 10 + (nptr[cur] - '0');
    x *= 10;
    cur++;
    consumed = cur;
  }
  retval = sign * (int_part + (double)fractional_part / x);

cleanup:
  if (endptr != NULL) {
    *endptr = nptr + consumed;
  }
  return retval;
}