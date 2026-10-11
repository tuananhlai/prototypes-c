#include "../dynamic_string.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_IMPLEMENTATION
#include "../arena.h"
#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"

typedef enum {
  PUSH,
  POP,
  ADD,
  SUB,
  CONSTANT,
  LOCAL,
  ARGUMENT,
  THIS,
  THAT,
  TEMP,
  NUMBER,
} TokenKind;

typedef struct {
  TokenKind kind;
  const char *val;
  size_t val_len;
} Token;

typedef struct {
  Token tokens[10];
  size_t tokens_len;
  Arena arena;
} TokenList;

TokenList *tokenlist_create() {
  TokenList *tl = malloc(sizeof(TokenList));
  return tl;
}

/**
 * Append a new token with the given params. `val` is copied.
 */
void tokenlist_append(TokenList *tl, TokenKind kind, size_t val_len,
                      const char val[val_len]) {
  char *copied = arena_alloc(&tl->arena, sizeof(char) * val_len);
  memcpy(copied, val, sizeof(char) * val_len);
  Token token = {
      .kind = kind,
      val = copied,
      val_len,
  };
  // FIXME: may overflow.
  tl->tokens[tl->tokens_len] = token;
  tl->tokens_len++;
}

void tokenlist_clear(TokenList *tl) {
  tl->tokens_len = 0;
  arena_reset(&tl->arena);
}

int tokenize(size_t n, const char vm_line[n], TokenList *out_token_list) {
  size_t i = 0;
  size_t len = 0;

  while (i < n) {
    while (i < n && isspace(vm_line[i]))
      i++;
    while (i + len < n && !isspace(vm_line[i + len]))
      len++;
    if (len > 0) {
      fwrite(vm_line + i, sizeof(char), len, stdout);
      putchar('\n');
    }
    i += len;
    len = 0;
  }

  return -1;
}

int parse(size_t n, Token, String *out_assembly) { return -1; }

/**
 * @brief Translate the given line of vm code to equivalent hack assembly.
 */
int translate(char *vm_line, String *out_assembly) { return -1; }

int main(void) {
  tokenize(16, "  hello, world! ", nullptr);
  return EXIT_SUCCESS;
}