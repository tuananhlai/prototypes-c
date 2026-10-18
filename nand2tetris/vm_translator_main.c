#include "vm_translator.c"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[argc]) {
  if (argc != 2) {
    fprintf(stderr, "expect 1 argument, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  Arena scratch;
  const char *vm_file_path = arena_sprintf(&scratch, "%s.vm", argv[1]);
  const char *asm_file_path = arena_sprintf(&scratch, "%s.asm", argv[1]);

  FILE *infile = fopen(vm_file_path, "r");
  FILE *outfile = fopen(asm_file_path, "w");

  if (!infile) {
    perror(vm_file_path);
    goto cleanup;
  }
  if (!outfile) {
    perror(asm_file_path);
    goto cleanup;
  }

  if (translate("Test", infile, outfile) != 0) {
    fprintf(stderr, "translating %s: %s", vm_file_path, error_get());
  }

cleanup:
  if (outfile)
    fclose(outfile);
  if (infile)
    fclose(infile);
  arena_free(&scratch);

  return EXIT_SUCCESS;
}