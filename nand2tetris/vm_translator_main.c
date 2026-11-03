#define _DEFAULT_SOURCE
#include "vm_translator.c"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

void path_basename(const char *file_path, Arena *arena, char **basename,
                   char **ext) {
  char *file_name = nullptr;
  path_split(file_path, arena, nullptr, &file_name);
  size_t n = strlen(file_name);
  size_t dot_idx = n;

  // . does not indicate extension if it was the first character.
  for (size_t i = 1; i < n; i++) {
    if (file_name[i] == '.')
      dot_idx = i;
  }

  if (basename) {
    *basename = arena_alloc(arena, dot_idx);
    (*basename)[dot_idx] = '\0';
    memcpy(*basename, file_name, dot_idx);
  }
  if (ext) {
    if (dot_idx >= n - 1)
      *ext = "";
    size_t ext_len = n - dot_idx;
    *ext = arena_alloc(arena, ext_len + 1);
    (*ext)[ext_len] = '\0';
    memcpy(ext, file_name + dot_idx - 1, ext_len);
  }
}

int list_vm_files(const char *path, Arena *arena, char **out_ls) {
  DIR *dir = opendir(path);
  if (dir == nullptr)
    return -1;

  struct FilePathList {
    char *items;
    size_t count;
    size_t capacity;
  };
  struct FilePathList ls = {0};

  struct dirent *entry;
  while (true) {
    entry = readdir(dir);
    if (entry == nullptr)
      break;
    if (entry->d_type == DT_REG) {
      char *ext = nullptr;
      path_basename(entry->d_name, arena, nullptr, &ext);
      if (strcmp(ext, "vm") == 0) arena_da_append(arena, &ls, )
    }
  }

  closedir(dir);
}

int main(int argc, char *argv[argc]) {
  if (argc != 2) {
    fprintf(stderr, "expect 1 argument, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  const char *vm_input_path = argv[1];
  struct stat st;
  if (stat(vm_input_path, &st)) {
    perror("reading stat");
    return EXIT_FAILURE;
  }

  Arena scratch = {0};

  if (S_ISDIR(st.st_mode)) {
    char *vm_file_paths = nullptr;
    list_vm_files(vm_input_path, &scratch, &vm_file_paths);
    goto cleanup;
  } else if (S_ISREG(st.st_mode)) {
  }

  char *vm_file_dir = nullptr;
  char *vm_file_name = nullptr;
  path_split(vm_input_path, &scratch, &vm_file_dir, &vm_file_name);

  char *vm_file_name_no_ext = nullptr;
  path_basename(vm_input_path, &scratch, &vm_file_name_no_ext);
  const char *asm_file_path =
      arena_sprintf(&scratch, "%s%s.asm", vm_file_dir, vm_file_name_no_ext);

  FILE *infile = fopen(vm_input_path, "r");
  FILE *outfile = fopen(asm_file_path, "w");

  if (!infile) {
    perror(vm_input_path);
    goto cleanup;
  }
  if (!outfile) {
    perror(asm_file_path);
    goto cleanup;
  }

  if (translate(vm_file_name_no_ext, infile, outfile) != 0) {
    fprintf(stderr, "translating %s: %s", vm_input_path, error_get());
  }

  if (outfile)
    fclose(outfile);
  if (infile)
    fclose(infile);
cleanup:
  arena_free(&scratch);

  return EXIT_SUCCESS;
}
