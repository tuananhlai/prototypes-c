#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t gcd(size_t a, size_t b);

typedef struct rat rat;

struct rat {
  bool sign;
  size_t num;
  size_t denom;
};

/* Functions that return a value of type rat. */
rat rat_get(signed sign, size_t num, size_t denom);
rat rat_get_normal(rat x);
rat rat_get_extended(rat x, size_t f);
rat rat_get_prod(rat x, rat y);
rat rat_get_sum(rat x, rat y);

/* Functions that operate on pointers to rat. */
void rat_destroy(rat *rp);
rat *rat_init(rat *rp, signed sign, size_t num, size_t denom);
rat *rat_normalize(rat *rp);
rat *rat_extend(rat *rp, size_t f);
rat *rat_sumup(rat *rp, rat y);
rat *rat_rma(rat *rp, rat x, rat y);

/* Functions that are implemented as exercises. */
/** @brief Print @a x into @a tmp and return tmp. **/
char const *rat_print(size_t len, char tmp[len], rat const *x);

/** @brief Print @a x normalize and print. **/
char const *rat_normalize_print(size_t len, char tmp[len], rat const *x);

rat *rat_dotproduct(rat rp[static 1], size_t n, rat const A[n], rat const B[n]);

rat rat_get_normal(rat x) {
  size_t c = gcd(x.num, x.denom);
  x.num /= c;
  x.denom /= c;
  return x;
}

char const *rat_print(size_t len, char out[len], rat const *x) {
  char tmp[len];

  int n =
      snprintf(tmp, len, "%c%zu/%zu", x->sign ? '+' : '-', x->num, x->denom);
  assert(n >= 0); // invalid encoding should not be possible
  if ((size_t)n + 1 > len) {
    return nullptr;
  }
  memcpy(out, tmp, len);

  return out;
}

char const *rat_normalize_print(size_t len, char out[len], rat const *x) {
  auto y = rat_get_normal(*x);
  return rat_print(len, out, &y);
}

rat *rat_dotproduct(rat rp[static 1], size_t n, rat const A[n],
                    rat const B[n]) {
  for (size_t i = 0; i < n; i++) {
    auto prod = rat_get_prod(A[i], B[i]);
    rat_sumup(rp, prod);
  }

  return rp;
}

size_t gcd(size_t a, size_t b) {
  size_t tmp;
  if (a < b) {
    tmp = a;
    a = b;
    b = tmp;
  }

  while (true) {
    if (a % b == 0) {
      return b;
    }
    tmp = b;
    b = a % b;
    a = tmp;
  }
}

/**
[Exs 8]Implement function rat_print as declared in listing 10.2.1. This function
should use-> members of its rat* argument. The printout should have the form
±nom/denum.

[Exs 9]Implement rat_print_normalized by combining rat_normalize and
rat_print.
*/
int main(void) {
  struct rat x = {
      .num = 4,
      .denom = 28,
      .sign = 0,
  };
  char out[60];
  auto res = rat_print(sizeof out, out, &x);
  if (!res) {
    puts("out char array has insufficient length");
  } else {
    puts(res);
  }

  res = rat_normalize_print(sizeof out, out, &x);
  if (!res) {
    puts("out char array has insufficient length");
  } else {
    puts(res);
  }

  return EXIT_SUCCESS;
}