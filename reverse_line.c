#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

void reverse(char arr[]);
ssize_t readline(FILE* fp, char** lineptr, size_t* capacity);

int main(int argc, char* argv[]) {
  if (argc == 1) {
    puts("usage: reverse_line <src> <dest?>");
    return EXIT_FAILURE;
  }

  FILE* src = fopen(argv[1], "r");
  if (src == NULL) {
    perror("error opening src");
    return EXIT_FAILURE;
  }

  FILE* dest;
  if (argc >= 3) {
    dest = fopen(argv[2], "w");
  } else {
    dest = stdout;
  }
  if (dest == NULL) {
    perror("error opening dest");
    return EXIT_FAILURE;
  }

  char* line = NULL;
  size_t capacity = 0;
  ssize_t length = 0;
  while (true) {
    length = readline(src, &line, &capacity);
    if (length == EOF) {
      break;
    }
    reverse(line);
    fputs(line, dest);
    fputc('\n', dest);
  }

  free(line);
  fclose(dest);
  fclose(src);
  return EXIT_SUCCESS;
}

/**
 * @brief Reverse the given string directly.
 */
void reverse(char arr[]) {
  const size_t n = strlen(arr);

  size_t j;
  char tmp;
  for (size_t i = 0; i < n / 2; i++) {
    j = n - 1 - i;
    tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

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
