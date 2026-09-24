#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  buf_max = 32,
};

/**
[Exs 4]Under what circumstances will this program finish with success or failure
return codes?

[Exs 5]Surprisingly, this program even works for files with lines
that have more than 31 characters. Why?

[Exs 6]Have the program read from stdin if no command-line argument is given.

[Exs 7]Have the program precede all output lines with line numbers if the first
command-line argument is "-n".
*/
int main(int argc, char *argv[argc + 1]) {
  int ret = EXIT_FAILURE;
  char buffer[buf_max] = {};
  bool display_line_num = true;

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

  size_t line_num;
  // Whether the previously read buffer ends in an EOL character.
  bool prev_buffer_eol;
  for (size_t i = 0; i < instreams_len; i++) {
    line_num = 1;
    prev_buffer_eol = true;
    while (fgets(buffer, buf_max, instreams[i])) {
      if (display_line_num && prev_buffer_eol) {
        fprintf(stdout, "     %zu  ", line_num);
        line_num++;
      }
      fputs(buffer, stdout);
      prev_buffer_eol =
          strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n';
    }
    fclose(instreams[i]);
    ret = EXIT_SUCCESS;
  }
  free(instreams);
  return ret;
}