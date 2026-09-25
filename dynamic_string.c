#include "dynamic_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void s_realloc_if_needed(String *s, size_t new_len);

String s_new(void) { return s_init("", 0); }

String s_init(const char *val, size_t len) {
  String s = {.data = NULL, .len = 0, .cap = 0};
  s_set(&s, val, len);
  return s;
}

void s_set(String *s, const char *val, size_t len) {
  s_realloc_if_needed(s, len);
  memcpy(s->data, val, len);
  s->data[len] = '\0';
  s->len = len;
}

void s_copy(String *dest, String *src) { s_set(dest, src->data, src->len); }

/** Append the given character to String s. */
void s_appendc(String *s, char ch) {
  size_t new_len = s->len + 1;
  s_realloc_if_needed(s, new_len);
  s->data[new_len - 1] = ch;
  s->data[new_len] = '\0';
  s->len = new_len;
}

void s_append(String *s, const char *val) {
  size_t val_len = strlen(val);
  size_t new_len = s->len + val_len;
  s_realloc_if_needed(s, new_len);
  memcpy(s->data + s->len, val, val_len);
  s->data[new_len] = '\0';
  s->len = new_len;
}

void s_trim(String *s) {
  size_t start = 0;
  size_t end = s->len;

  while (start < s->len && s->data[start] == ' ') {
    start++;
  }
  while (end > start && s->data[end - 1] == ' ') {
    end--;
  }

  size_t new_len = end - start;
  memmove(s->data, s->data + start, new_len);
  s->data[new_len] = '\0';
  s->len = new_len;
}

void s_clear(String *s) { s_set(s, "", 0); }

void s_substr(String *s, size_t start, size_t end, String *sub_str) {
  if (end > s->len)
    end = s->len;
  if (start < 0)
    start = 0;

  if (start >= end) {
    s_clear(sub_str);
    return;
  }

  s_set(sub_str, s->data + start, end - start);
}

void s_trim_suffix(String *s, const char *suffix) {
  size_t suffix_len = strlen(suffix);
  if (s->len < suffix_len) {
    return;
  }

  bool has_suffix = true;
  size_t start = s->len - suffix_len;
  for (size_t i = start; i < s->len; i++) {
    if (s->data[i] != suffix[i - start]) {
      has_suffix = false;
      break;
    }
  }

  if (!has_suffix) {
    return;
  }

  s->len = s->len - suffix_len;
  s->data[s->len] = '\0';
}

void s_destroy(String *s) {
  free(s->data);
  s->data = NULL;
  s->len = 0;
  s->cap = 0;
}

/**
 * @brief Rellocate the given string s if necessary to accomodate a string with
 * the given length.
 */
static void s_realloc_if_needed(String *s, size_t new_len) {
  if (new_len + 1 <= s->cap) {
    return;
  }

  // Add 1 to new_len before doubling so that 0-length strings can grow too.
  s->cap = (new_len + 1) * 2;
  s->data = realloc(s->data, s->cap * sizeof(char));
}
