#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: cpu <string>\n");
    exit(EXIT_FAILURE);
  }
  char* str = argv[1];
  while (true) {
    Spin(1);
    printf("%s\n", str);
  }
  return EXIT_SUCCESS;
}