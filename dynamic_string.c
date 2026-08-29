#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} String;

String s_create(const char* val, size_t len) {
  size_t cap = len + 1;
  char* data = malloc(cap * sizeof(char));
  String s = {.data = data, .len = len, .cap = cap};
  for (size_t i = 0; i < len; i++) {
    s.data[i] = val[i];
  }
  s.data[len] = '\0';
  return s;
}

static void s_realloc_if_needed(String* s, size_t new_len) {
  if (new_len + 1 <= s->cap) {
    return;
  }

  s->cap = new_len * 2;
  s->data = realloc(s->data, s->cap * sizeof(char));
}

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

void s_destroy(String s) { free(s.data); }

int main(void) {
  char* init_data = "Hello, World!";
  String s = s_create(init_data, strlen(init_data));
  char* concat_data = " Greeting!";
  s_concat(&s, concat_data, strlen(concat_data));
  char arr[] = {'a', 'b', 'c'};
  for (size_t i = 0; i < 3; i++) {
    s_add(&s, arr[i]);
  }
  puts(s.data);
  s_destroy(s);
  return EXIT_SUCCESS;
}