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
} TokenMapEntry;

TokenMapEntry *tm_create(void) {
  TokenMapEntry *retval = nullptr;
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

const Token *tm_get(TokenMapEntry *tm, const char *keyword) {
  TokenMapEntry *retval = shgetp_null(tm, keyword);
  if (retval == nullptr) {
    return nullptr;
  }
  return &retval->value;
}

void tm_destroy(TokenMapEntry *tm) { shfree(tm); }

typedef struct {
  Token *items;
  size_t count;
  size_t capacity;
} TokenList;

void tokenlist_append(Arena *arena, TokenList *tl, Token token) {
  arena_da_append(arena, tl, token);
}

typedef struct {
  TokenMapEntry *builtin_tokens;
} Lexer;

void lexer_init(Lexer *lexer) { lexer->builtin_tokens = tm_create(); }

void lexer_deinit(Lexer *lexer) { tm_destroy(lexer->builtin_tokens); }

static int resolve_word(Lexer *lexer, Arena *arena, const char *word,
                        Token *out_token) {
  size_t n = strlen(word);
  if (n == 0) {
    return -1;
  }

  const Token *builtin_token = tm_get(lexer->builtin_tokens, word);
  if (builtin_token) {
    *out_token = *builtin_token;
    return 0;
  }

  if (isdigit(word[0])) {
    for (size_t i = 1; i < n; i++) {
      if (!isdigit(word[i]))
        return -1;
    }

    char *val = arena_alloc(arena, sizeof(char) * n);
    memcpy(val, word, n);
    *out_token = token_create3(TK_NUMBER, n, val);

    return 0;
  }

  return -1;
}

int lexer_tokenize(Lexer *lexer, Arena *arena, size_t n, const char vm_line[n],
                   TokenList *out_tokens) {
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

    char word[len + 1];
    memcpy(word, vm_line + i, len);
    word[len] = '\0';

    Token token = {0};
    int res = resolve_word(lexer, arena, word, &token);
    if (res != 0) {
      return -1;
    }
    tokenlist_append(arena, out_tokens, token);

    i += len;
    len = 0;
  }

  return 0;
}

int main(void) {
  int status = EXIT_SUCCESS;

  const char *lines[] = {" push constant  16", " pop local "};
  // Allocated memory when processing a single line of source VM code, so it
  // can be reseted afterward.
  Arena scratch = {0};
  Lexer lexer = {0};
  lexer_init(&lexer);

  for (size_t i = 0; i < sizeof(lines) / sizeof(*lines); i++) {
    TokenList tokens = {0};
    const char *line = lines[i];
    int res = lexer_tokenize(&lexer, &scratch, strlen(line), line, &tokens);
    if (res < 0) {
      puts("error cannot tokenize");
      status = EXIT_FAILURE;
      break;
    }

    puts(line);
    for (size_t j = 0; j < tokens.count; j++) {
      Token item = tokens.items[j];
      printf("%d", item.kind);
      if (item.val_len > 0) {
        putchar(' ');
        fwrite(item.val, sizeof(char), item.val_len, stdout);
      }
      putchar('\n');
    }

    arena_reset(&scratch);
  }

  lexer_deinit(&lexer);
  arena_free(&scratch);

  return status;
}