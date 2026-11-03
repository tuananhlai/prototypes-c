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
int list_vm_files(const char *dir_path, Arena *arena, size_t *out_ls_len,
                  FILE ***out_ls) {
  DIR *dir = opendir(dir_path);
  if (dir == nullptr)
    return -1;

  struct FileList {
    FILE **items;
    size_t count;
    size_t capacity;
  };
  struct FileList ls = {0};

  int retval = -1;
  char file_path[1024];
  struct dirent *entry;
  bool fopen_all_succeeded = true;
  while (true) {
    entry = readdir(dir);
    if (entry == nullptr)
      break;
    if (entry->d_type == DT_REG) {
      char *ext = nullptr;
      file_basename(entry->d_name, arena, nullptr, &ext);
      if (strncmp(ext, "vm", 2) == 0) {
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path,
                 entry->d_name);
        FILE *input_file = fopen(file_path, "r");
        if (!input_file) {
          fopen_all_succeeded = false;
          goto cleanup;
        }
      }
    }
  }

  *out_ls = ls.items;
  *out_ls_len = ls.count;
  retval = 0;
cleanup:
  if (!fopen_all_succeeded) {
    for (size_t i = 0; i < ls.count; i++) {
      fclose(ls.items[i]);
    }
  }
  closedir(dir);
  return retval;
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
  FILE **in_files = nullptr;
  size_t in_files_len = 0;
  FILE *out_file = nullptr;
  char *static_var_prefix = nullptr;
  int res;

  if (S_ISDIR(st.st_mode)) {
    res = list_vm_files(vm_input_path, &scratch, &in_files_len, &in_files);
    if (res != 0)
      goto cleanup;
  } else if (S_ISREG(st.st_mode)) {
    FILE *in_file = fopen(vm_input_path, "r");
    if (!in_file) {
      perror("open single file");
      goto cleanup;
    }

    in_files = arena_alloc(&scratch, sizeof(FILE *));
    in_files[0] = in_file;
    in_files_len = 1;

    char *vm_file_dir = nullptr;
    char *vm_file_name = nullptr;
    path_split(vm_input_path, &scratch, &vm_file_dir, &vm_file_name);
    char *vm_file_name_no_ext = nullptr;
    file_basename(vm_file_name, &scratch, &vm_file_name_no_ext, nullptr);
    const char *asm_file_path =
        arena_sprintf(&scratch, "%s%s.asm", vm_file_dir, vm_file_name_no_ext);
    out_file = fopen(asm_file_path, "w");
    if (!out_file) {
      perror("open asm file for writing");
      goto cleanup;
    }
    static_var_prefix = vm_file_name_no_ext;
  } else {
    fprintf(stderr, "invalid input: expected directory or file, got %d\n",
            st.st_mode);
    goto cleanup;
  }

  for (size_t i = 0; i < in_files_len; i++) {
    if (translate(static_var_prefix, in_files[i], out_file) != 0) {
      fprintf(stderr, "translating %s: %s", vm_input_path, error_get());
    }
  }

cleanup:
  if (out_file)
    fclose(out_file);
  for (size_t i = 0; i < in_files_len; i++) {
    fclose(in_files[i]);
  }
  arena_free(&scratch);

  return EXIT_SUCCESS;
}
