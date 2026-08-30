#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} String;

/** @brief Rellocate the given string s if necessary to accomodate a string with the given length. */
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

String s_new() { return s_init("", 0); }

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

void s_destroy(String* s) {
  free(s->data);
  s->data = NULL;
}

/**
 * @brief Read a line from a file stream into a String.
 * @return EOF (-1) if we reached EOF and no data were read. 0 otherwise.
 */
int readline(FILE* fp, String* s) {
  s_set(s, "", 0);

  int ch;
  while (true) {
    ch = fgetc(fp);
    if (ch == '\n' || ch == EOF) {
      break;
    }
    s_add(s, ch);
  }

  if (s->len == 0 && ch == EOF) return EOF;

  return 0;
}

typedef enum {
  A_INSTRUCTION,
  C_INSTRUCTION,
  L_INSTRUCTION,
} InstructionType;

typedef struct {
  FILE* f;
  String cur_ins;
  bool has_more;
} Parser;

Parser* parser_open(FILE* f) {
  Parser* p = malloc(sizeof(Parser));
  p->f = f;
  p->has_more = true;
  p->cur_ins = s_new();
  return p;
}

bool parser_has_more_lines(Parser* p) { return p->has_more; }

void parser_advance(Parser* p) {
  while (true) {
    int res = readline(p->f, &p->cur_ins);
    if (res == EOF) {
      p->has_more = false;
      return;
    }
    s_trim(&p->cur_ins);
    if (p->cur_ins.len != 0) {
      break;
    }
  }
}

InstructionType parser_instruction_type(Parser* p) {
  switch (p->cur_ins.data[0]) {
    case '@':
      return A_INSTRUCTION;
    case '(':
      return L_INSTRUCTION;
    default:
      return C_INSTRUCTION;
  }
}

int parser_symbol(Parser* p, String* s) {
  if (parser_instruction_type(p) != A_INSTRUCTION) {
    return -1;
  }

  s_set(s, "", 0);

  size_t pos = 0;
  if (p->cur_ins.data[pos] != '@') {
    return -1;
  }
  pos++;

  while (pos < p->cur_ins.len && p->cur_ins.data[pos] >= '0' &&
         p->cur_ins.data[pos] <= '9') {
    s_add(s, p->cur_ins.data[pos]);
    pos++;
  }

  return 0;
}

void parser_destroy(Parser* p) {
  s_destroy(&p->cur_ins);
  free(p);
}

int main(void) {
  const char* text = "   \n @10  \n@2023 \n";
  FILE* f = fmemopen((void*)text, strlen(text), "r");

  Parser* p = parser_open(f);

  String symbol = s_new();
  parser_advance(p);
  parser_symbol(p, &symbol);
  puts(symbol.data);

  parser_advance(p);
  parser_symbol(p, &symbol);
  puts(symbol.data);

  // Clean up.
  s_destroy(&symbol);
  parser_destroy(p);
  fclose(f);
  return EXIT_SUCCESS;
}