#include <stdio.h>
#include <stdlib.h>

#define CORVID_NAME                                                            \
  (char const *const[corvid_num]) {                                            \
    [raven] = "raven", [magpie] = "magpie", [jay] = "jay",                     \
  }

int main(void) {
  enum corvid : int {
    magpie,
    raven,
    jay,
    corvid_num,
  };

  for (unsigned i = 0; i < corvid_num; i++) {
    printf("Corvid %u is the %s\n", i, CORVID_NAME[i]);
  }

  return EXIT_SUCCESS;
}