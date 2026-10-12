#include "../dynamic_string.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_IMPLEMENTATION
#include "../arena.h"

typedef enum {
  TK_PUSH,
  TK_POP,
  TK_ADD,
  TK_SUB,
  TK_CONSTANT,
  TK_LOCAL,
  TK_ARGUMENT,
  TK_THIS,
  TK_THAT,
  TK_TEMP,
  TK_NUMBER,
} TokenKind;

typedef struct {
  TokenKind kind;
  const char *val;
  size_t val_len;
} Token;

Token token_create(TokenKind kind) { return (Token){.kind = kind}; }

Token token_create3(TokenKind kind, size_t val_len, const char val[val_len]) {
  return (Token){
      .kind = kind,
      .val = val,
      .val_len = val_len,
  };
}

typedef struct {
  char *key;
  Token value;
} TokenMap;

TokenMap *tm_create() {
  TokenMap *retval = nullptr;
  shput(retval, "push", token_create(TK_PUSH));
  shput(retval, "pop", token_create(TK_POP));
  shput(retval, "add", token_create(TK_ADD));
  shput(retval, "sub", token_create(TK_SUB));
  shput(retval, "constant", token_create(TK_CONSTANT));
  shput(retval, "local", token_create(TK_LOCAL));
  shput(retval, "argument", token_create(TK_ARGUMENT));
  shput(retval, "this", token_create(TK_THIS));
  shput(retval, "that", token_create(TK_THAT));
  shput(retval, "temp", token_create(TK_TEMP));
  return retval;
}

Token *tm_get(TokenMap *tm, const char *keyword) {
  auto retval = shgetp_null(tm, keyword);
  if (retval == nullptr) {
    return nullptr;
  }
  return &retval->value;
}

void tm_destroy(TokenMap *tm) { shfree(tm); }

typedef struct {
  Token *tokens;
  size_t len;
  size_t cap;
  Arena *arena;
} TokenList;

TokenList *tokenlist_new(Arena *arena) {
  TokenList *tl = arena_alloc(arena, sizeof(TokenList));
  tl->tokens = nullptr;
  tl->len = 0;
  tl->cap = 0;
  tl->arena = arena;
  return tl;
}

void tokenlist_append(TokenList *tl, Token token) {
  if (tl->len == tl->cap) {
    size_t new_cap = (tl->cap + 1) * 2;
    tl->tokens =
        arena_realloc(tl->arena, tl->tokens, sizeof(*tl->tokens) * tl->cap,
                      sizeof(*tl->tokens) * new_cap);
    tl->cap = new_cap;
  }
  tl->tokens[tl->len] = token;
  tl->len++;
}

typedef struct {
  TokenMap *builtin_tokens;
} Lexer;

void lexer_init(Lexer *lexer) { lexer->builtin_tokens = tm_create(); }

void lexer_deinit(Lexer *lexer) { tm_destroy(lexer->builtin_tokens); }

int lexer_tokenize(Lexer *lexer, size_t n, const char vm_line[n],
                   TokenList *out_tokenlist) {
  size_t i = 0;
  size_t len = 0;

  while (i < n) {
    while (i < n && isspace(vm_line[i]))
      i++;
    while (i + len < n && !isspace(vm_line[i + len]))
      len++;
    if (len == 0) {
      break;
    }

    char key[len + 1];
    memcpy(key, vm_line + i, len);
    key[len] = '\0';
    auto builtin_token = tm_get(lexer->builtin_tokens, key);
    if (builtin_token) {
      tokenlist_append(out_tokenlist, *builtin_token);
    } else {
      return -1;
    }

    i += len;
    len = 0;
  }

  return 0;
}

int parse(size_t n, Token, String *out_assembly) { return -1; }

/**
 * @brief Translate the given line of vm code to equivalent hack assembly.
 */
int translate(char *vm_line, String *out_assembly) { return -1; }

int main(void) {
  Arena def = {0};

  Lexer lexer = {0};
  lexer_init(&lexer);

  auto tokenlist = tokenlist_new(&def);
  auto res =
      lexer_tokenize(&lexer, 26, "  push constant temp this ", tokenlist);
  if (res < 0) {
    puts("error cannot tokenize");
    goto cleanup;
  }

  for (size_t i = 0; i < tokenlist->len; i++) {
    printf("%d\n", tokenlist->tokens[i].kind);
  }

cleanup:
  lexer_deinit(&lexer);
  arena_free(&def);

  return EXIT_SUCCESS;
}