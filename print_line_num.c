#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

ssize_t readline(FILE* fp, char** lineptr, size_t* capacity);

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "invalid number of arguments: %d\n", argc);
    return EXIT_FAILURE;
  }

  size_t target_line_num = atoi(argv[2]);
  FILE* fp = fopen(argv[1], "r");

  char* line;
  size_t capacity = 0;
  for (size_t i = 0; i < target_line_num; i++) {
    readline(fp, &line, &capacity);
  }

  puts(line);
  free(line);
  fclose(fp);
  return EXIT_SUCCESS;
}

ssize_t readline(FILE* fp, char** lineptr, size_t* capacity) {
  if (lineptr == NULL || *capacity == 0) {
    *capacity = 128;
    *lineptr = malloc(*capacity * sizeof(char));
  }

  int ch;
  size_t length = 0;
  while (true) {
    if (length + 1 >= *capacity) {
      *capacity *= 2;
      *lineptr = realloc(*lineptr, *capacity);
    }
    ch = fgetc(fp);
    if (ch == '\n') {
      (*lineptr)[length] = '\0';
      break;
    }
    (*lineptr)[length] = ch;
    length++;
  }

  if (length == 0) return -1;

  return (ssize_t)length;
}
