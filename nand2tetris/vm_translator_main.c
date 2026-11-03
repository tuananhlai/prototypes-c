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

void file_basename(const char *file_name, Arena *arena, char **basename,
                   char **ext) {
  size_t n = strlen(file_name);
  size_t last_dot_idx = n;

  // . does not indicate extension if it was the first character.
  for (size_t i = 1; i < n; i++) {
    if (file_name[i] == '.')
      last_dot_idx = i;
  }

  if (basename) {
    *basename = arena_alloc(arena, last_dot_idx);
    (*basename)[last_dot_idx] = '\0';
    memcpy(*basename, file_name, last_dot_idx);
  }
  if (ext) {
    if (last_dot_idx >= n - 1) {
      *ext = "";
    } else {
      size_t ext_len = n - last_dot_idx;
      *ext = arena_alloc(arena, ext_len + 1);
      (*ext)[ext_len] = '\0';
      memcpy(*ext, file_name + last_dot_idx + 1, ext_len);
    }
  }
}

/**
 * List all files with extension .vm from the given directory. The output string
 * array will be allocated from the given arena allocator.
 */
int list_vm_files(DIR *dir, Arena *arena, size_t *out_ls_len, char ***out_ls) {
  struct FilePathList {
    char **items;
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
      file_basename(entry->d_name, arena, nullptr, &ext);
      if (strncmp(ext, "vm", 2) == 0) {
        arena_da_append(arena, &ls, entry->d_name);
      }
    }
  }

  *out_ls = ls.items;
  *out_ls_len = ls.count;
  return 0;
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
  struct {
    FILE **items;
    size_t count;
    size_t capacity;
  } in_files;

  if (S_ISDIR(st.st_mode)) {
    DIR *dir = opendir(vm_input_path);
    if (dir == nullptr) {
      closedir(dir);
      goto cleanup;
    }

    char **vm_file_paths = nullptr;
    size_t vm_file_path_len = 0;
    list_vm_files(dir, &scratch, &vm_file_path_len, &vm_file_paths);

    closedir(dir);
    goto cleanup;
  } else if (S_ISREG(st.st_mode)) {
  }

  char *vm_file_dir = nullptr;
  char *vm_file_name = nullptr;
  path_split(vm_input_path, &scratch, &vm_file_dir, &vm_file_name);

  char *vm_file_name_no_ext = nullptr;
  file_basename(vm_file_name, &scratch, &vm_file_name_no_ext, nullptr);
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
