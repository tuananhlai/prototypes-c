#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static_assert('z' - 'a' == 25, "alphabetic characters not contiguous");

static const char *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

unsigned hexatridecimal(int a) {
  if (isdigit(a)) {
    return a - '0';
  }

  a = toupper(a);
  char *p = strchr(alphabet, a);
  return isupper(a) ? p - alphabet : -1;
}

/**
[Exs 10]The second return of hexatridecimal makes an assumption about the
relation between a and’A’ What is it?

[Exs 11]Describe an error scenario in which this assumption is not fulfilled.

[Exs 12]Fix this bug: that is, rewrite this code such that it makes no
assumption about the relation between a and ’A’
.
*/
int main(void) {
  printf("%u", hexatridecimal('j'));
  return EXIT_SUCCESS;
}