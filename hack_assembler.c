#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Read a line from a file stream into a heap-allocated buffer.
 * The given char* pointer must be free if it's nolonger used.
 * @return The line length (not including terminating null character), or -1 if
 * we reached EOF.
 */
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
    if (ch == '\n' || ch == EOF) {
      (*lineptr)[length] = '\0';
      break;
    }
    (*lineptr)[length] = ch;
    length++;
  }

  if (length == 0 && ch == EOF) return EOF;

  return (ssize_t)length;
}

typedef struct {
  FILE* f;
  char* current_instruction;
} Parser;

Parser* parser_open(FILE* f) {
  Parser* p = malloc(sizeof(Parser));
  p->f = f;
  return p;
}

void parser_advance(Parser* p) {
}

int main(void) { return EXIT_SUCCESS; }