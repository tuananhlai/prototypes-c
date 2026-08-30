#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} String;

static void s_realloc_if_needed(String* s, size_t new_len);
void s_set(String* s, const char* val, size_t len);
String s_init(const char* val, size_t len);
String s_new(void);
void s_copy(String* dest, String* src);
void s_add(String* s, char ch);
void s_concat(String* s, const char* val, size_t len);
void s_trim(String* s);
void s_clear(String* s);
void s_substr(String* s, size_t start, size_t end, String* sub_str);
void s_destroy(String* s);

/** @brief Rellocate the given string s if necessary to accomodate a string with
 * the given length. */
static void s_realloc_if_needed(String* s, size_t new_len) {
  if (new_len + 1 <= s->cap) {
    return;
  }

  s->cap = (new_len + 1) * 2;
  s->data = realloc(s->data, s->cap * sizeof(char));
}

void s_set(String* s, const char* val, size_t len) {
  s_realloc_if_needed(s, len);
  memcpy(s->data, val, len);
  s->data[len] = '\0';
  s->len = len;
}

String s_init(const char* val, size_t len) {
  String s = {.data = NULL, .len = 0, .cap = 0};
  s_set(&s, val, len);
  return s;
}

String s_new(void) { return s_init("", 0); }

void s_copy(String* dest, String* src) { s_set(dest, src->data, src->len); }

/** Append the given character to String s. */
void s_add(String* s, char ch) {
  size_t new_len = s->len + 1;
  s_realloc_if_needed(s, new_len);
  s->data[new_len - 1] = ch;
  s->data[new_len] = '\0';
  s->len = new_len;
}

void s_concat(String* s, const char* val, size_t len) {
  size_t new_len = s->len + len;
  s_realloc_if_needed(s, new_len);
  memcpy(s->data + s->len, val, len);
  s->data[new_len] = '\0';
  s->len = new_len;
}

void s_trim(String* s) {
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

void s_clear(String* s) { s_set(s, "", 0); }

void s_substr(String* s, size_t start, size_t end, String* sub_str) {
  s_set(sub_str, s->data + start, end - start);
}

void s_destroy(String* s) {
  free(s->data);
  s->data = NULL;
}

int main(void) {
  char* init_data = "Hello, World!";
  String s = s_init(init_data, strlen(init_data));
  char* concat_data = " Greeting!";
  s_concat(&s, concat_data, strlen(concat_data));
  char arr[] = {'a', 'b', 'c'};
  for (size_t i = 0; i < 3; i++) {
    s_add(&s, arr[i]);
  }
  puts(s.data);

  String padded = s_init("   trim me   ", 13);
  s_trim(&padded);
  puts(padded.data);

  String sub = s_new();
  s_substr(&s, 0, 5, &sub);
  puts(sub.data);

  s_destroy(&sub);
  s_destroy(&padded);
  s_destroy(&s);
  return EXIT_SUCCESS;
}
