#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

enum {
  buf_max = 32,
};

int main(int argc, char *argv[argc + 1]) {
  int ret = EXIT_FAILURE;
  char buffer[buf_max] = {};

  FILE **instreams;
  size_t instreams_len;
  if (argc == 1) {
    instreams_len = 1;
    instreams = malloc(sizeof(FILE *) * instreams_len);
    instreams[0] = stdin;
  } else {
    instreams_len = argc - 1;
    instreams = malloc(sizeof(FILE *) * instreams_len);
    for (int i = 1; i < argc; i++) {
      FILE *instream = fopen(argv[i], "r");
      if (!instream) {
        fprintf(stderr, "could not open %s:", argv[i]);
        perror(0);
        errno = 0;
      }
      instreams[i - 1] = instream;
    }
  }

  for (size_t i = 0; i < instreams_len; i++) {
    while (fgets(buffer, buf_max, instreams[i])) {
      fputs(buffer, stdout);
    }
    fclose(instreams[i]);
    ret = EXIT_SUCCESS;
  }
  free(instreams);
  return ret;
}