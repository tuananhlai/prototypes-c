#include <stdio.h>
#include <stdlib.h>

char* readline(FILE* fp);

int main(void) {
  size_t target_line_num = 8;

  FILE* fp = fopen("Taskfile.yml", "r");

  char* line;
  size_t current_line_num = 1;
  while (true) {
    line = readline(fp);
    if (current_line_num == target_line_num) {
      break;
    }
    free(line);
    current_line_num++;
  }

  puts(line);
  free(line);
  fclose(fp);
  return EXIT_SUCCESS;
}

char* readline(FILE* fp) {
  size_t length = 0;
  size_t capacity = 128;
  char* line = malloc(capacity * sizeof(char));

  int ch;
  while ((ch = fgetc(fp)) != EOF) {
    if (ch == '\n') {
      line[length] = '\0';
      break;
    }
    line[length] = ch;
    length++;
    if (length == capacity - 1) {
      capacity *= 2;
      line = realloc(line, capacity);
    }
  }

  if (length == 0) {
    free(line);
    return NULL;
  }

  return line;
}