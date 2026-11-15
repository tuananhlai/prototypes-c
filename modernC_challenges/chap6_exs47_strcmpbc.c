#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * A bound-checking version of `strcmp`. Comparison stops at `\0` if it's found
 * in the first `n` characters.
 */
int strcmpbc(const char *s1, const char *s2, size_t n) {
  char *end_s1 = memchr(s1, 0, n);
  char *end_s2 = memchr(s2, 0, n);

  if (end_s1) {
    size_t s1_len = (size_t)(end_s1 - s1) + 1;
    if (n > s1_len)
      n = s1_len;
  }
  if (end_s2) {
    size_t s2_len = (size_t)(end_s2 - s2) + 1;
    if (n > s2_len)
      n = s2_len;
  }

  return memcmp(s1, s2, n);
}
