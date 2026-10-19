#include "vm_translator.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void path_split(const char *file_path, Arena *arena, char **path,
                char **file_name) {
  size_t n = strlen(file_path);
  size_t start = n;
  while (start > 0 && file_path[start - 1] != '/') {
    start--;
  }

  size_t path_len = start;
  size_t file_name_len = n - start;

  if (path) {
    *path = arena_alloc(arena, path_len + 1);
    (*path)[path_len] = '\0';

    if (path_len)
      memcpy(*path, file_path, path_len);
  }
  if (file_name) {
    *file_name = arena_alloc(arena, file_name_len + 1);
    (*file_name)[file_name_len] = '\0';

    if (file_name_len)
      memcpy(*file_name, file_path + start, file_name_len);
  }
}

void path_basename(const char *file_path, Arena *arena, char **basename) {
  char *file_name = nullptr;
  path_split(file_path, arena, nullptr, &file_name);
  size_t n = strlen(file_name);
  size_t dot_idx = n;
  // FIXME
  while (dot_idx > 0 && file_name[dot_idx - 1] != '.') {
    dot_idx--;
  }
  dot_idx--;

  *basename = arena_alloc(arena, dot_idx);
  (*basename)[dot_idx] = '\0';
  memcpy(*basename, file_name, dot_idx);
}

int main(int argc, char *argv[argc]) {
  if (argc != 2) {
    fprintf(stderr, "expect 1 argument, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  Arena scratch;
  const char *vm_file_path = argv[1];
  char *vm_file_dir = nullptr;
  char *vm_file_name = nullptr;
  path_split(vm_file_path, &scratch, &vm_file_dir, &vm_file_name);

  char *vm_file_name_no_ext = nullptr;
  path_basename(vm_file_path, &scratch, &vm_file_name_no_ext);
  const char *asm_file_path =
      arena_sprintf(&scratch, "%s%s.asm", vm_file_dir, vm_file_name_no_ext);

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

  if (translate(vm_file_name_no_ext, infile, outfile) != 0) {
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