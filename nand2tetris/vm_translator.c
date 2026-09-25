#include "../dynamic_string.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define ARENA_IMPLEMENTATION
#include "../arena.h"

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
  Token *tokens;
} TokenList;

TokenList *tokenlist_create() {
  TokenList *tl = malloc(sizeof(TokenList));
  return tl;
}

void tokenlist_push(Token tok) {}

int tokenize(size_t n, const char vm_line[n], Arena *arena, TokenList *out_token_list) {
  char* number = arena_alloc(arena, sizeof(char) * 12);
  return -1;
}

int parse(size_t n, Token, String *out_assembly) { return -1; }

/**
Translate the given line of vm code to equivalent hack assembly.
*/
int translate(char *vm_line, String *out_assembly) { return -1; }

int main(void) { return EXIT_SUCCESS; }