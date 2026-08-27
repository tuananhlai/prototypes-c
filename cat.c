#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  FILE* fp;
  if (argc == 1) {
    fp = stdin;
  } else {
    fp = fopen(argv[1], "r");
  }

  int ch;

  if (fp == NULL) {
    perror("input.txt");
    return EXIT_FAILURE;
  }

  while (true) {
    ch = fgetc(fp);
    if (ch == EOF) {
      break;
    }
    putchar(ch);
  }

  fclose(fp);
  return EXIT_SUCCESS;
}