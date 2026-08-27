#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  uint32_t target_line_num = 6;

  FILE* fp = fopen("Taskfile.yml", "r");

  int ch;
  uint32_t current_line_num = 1;
  while (current_line_num < target_line_num) {
    ch = fgetc(fp);
    if (ch == '\n') {
      current_line_num++;
    }
  }

  ch = 0;
  while (ch != '\n') {
    ch = fgetc(fp);
    putchar(ch);
  }

  fclose(fp);
  return EXIT_SUCCESS;
}