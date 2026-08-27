#include <stdio.h>
#include <stdlib.h>

int main(void) {
  FILE *fp = fopen("Taskfile.yml", "r");
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