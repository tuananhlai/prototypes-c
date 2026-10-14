#include "../dynamic_string.h"
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_IMPLEMENTATION
#include "../arena.h"
#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"

static char g_err[256];

// Formats a message into the error buffer and returns -1, so failures can be
// reported with `return error_set("...", ...);`.
[[gnu::format(printf, 1, 2)]]
static int error_set(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(g_err, sizeof g_err, fmt, args);
  va_end(args);
  return -1;
}

// Returns the message of the most recent error.
static const char *error_get(void) { return g_err; }

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
  TK_STATIC,
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
  shput(retval, "static", token_create(TK_STATIC));
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
    return error_set("empty word");
  }

  const Token *builtin_token = tm_get(lexer->builtin_tokens, word);
  if (builtin_token) {
    *out_token = *builtin_token;
    return 0;
  }

  if (isdigit(word[0])) {
    for (size_t i = 1; i < n; i++) {
      if (!isdigit(word[i]))
        return error_set("invalid number '%s'", word);
    }

    char *val = arena_alloc(arena, sizeof(char) * n);
    memcpy(val, word, n);
    *out_token = token_create3(TK_NUMBER, n, val);

    return 0;
  }

  return error_set("unknown word '%s'", word);
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

typedef struct {
  size_t cur;
  const TokenList *tokens;
  const char *_file_name;
} Parser;

typedef enum {
  MS_ARGUMENT,
  MS_LOCAL,
  MS_STATIC,
  MS_CONSTANT,
  MS_THIS,
  MS_THAT,
  MS_POINTER,
  MS_TEMP,
} MemorySegment;

typedef struct {
  MemorySegment segment;
  size_t index;
} CommandPush;

void parser_init(Parser* p, const char* file_name) {
  p->cur = 0;
  p->tokens = nullptr;
  p->_file_name = file_name;
}

// Hack Assembly code which pushes the value stored inside the D register
// into the stack.
// TODO: how to check for stack overflow?
static const char *hack_asm_push = "@SP\nA=M\nM=D\n@SP\nM=M+1\n";

static int parser_read_segment(Parser *p, MemorySegment *out_ms) {
  if (p->cur >= p->tokens->count) {
    return error_set("expected memory segment, got end of line");
  }

  switch (p->tokens->items[p->cur].kind) {

  case TK_CONSTANT:
    *out_ms = MS_CONSTANT;
    break;
  case TK_LOCAL:
    *out_ms = MS_LOCAL;
    break;
  case TK_ARGUMENT:
    *out_ms = MS_ARGUMENT;
    break;
  case TK_THIS:
    *out_ms = MS_THIS;
    break;
  case TK_THAT:
    *out_ms = MS_THAT;
    break;
  case TK_TEMP:
    *out_ms = MS_TEMP;
    break;
  case TK_STATIC:
    *out_ms = MS_STATIC;
    break;
  default:
    return error_set("expected memory segment at token %zu", p->cur + 1);
  }

  p->cur++;
  return 0;
}

static int parser_read_number(Parser *p, size_t *out_num) {
  if (p->cur >= p->tokens->count)
    return error_set("expected number, got end of line");
  if (p->tokens->items[p->cur].kind != TK_NUMBER)
    return error_set("expected number at token %zu", p->cur + 1);

  Token token = p->tokens->items[p->cur];
  char num_str[token.val_len + 1];
  num_str[token.val_len] = '\0';
  memcpy(num_str, token.val, token.val_len);

  int num = atoi(num_str);
  if (num < 0)
    return error_set("invalid number '%s'", num_str);

  *out_num = num;
  p->cur++;
  return 0;
}

static int parser_read_eol(Parser *p) {
  if (p->cur < p->tokens->count) {
    return error_set("expected eol, got token kind %d",
                     p->tokens->items[p->cur].kind);
  }

  return 0;
}

// Maximum number of digit a string converted from size_t can have.
#define SIZE_T_STR_MAX_DIGITS 21

// Convert size_t to string.
static int to_str(size_t num, size_t str_size,
                  char str[static SIZE_T_STR_MAX_DIGITS]) {
  int n = snprintf(str, str_size, "%zu", num);
  if (n < 0 || (size_t)n >= str_size) {
    error_set("error converting int to str: %zu", num);
    return -1;
  }
  return 0;
}

#define TEMP_ADDR 5

static int command_push_translate(Parser *p, CommandPush cmd, String *out) {
  // TODO: validate size of `index`
  // max number of digit for 64-bit uint
  char index_str[SIZE_T_STR_MAX_DIGITS];
  int res = to_str(cmd.index, sizeof(index_str), index_str);
  if (res < 0) {
    return -1;
  }

  switch (cmd.segment) {
  case MS_ARGUMENT:
    s_append(out, "@ARG\nD=M\n@");
    s_append(out, index_str);
    s_append(out, "\nA=D+A\nD=M\n");
    s_append(out, hack_asm_push);
    return 0;
  case MS_LOCAL:
    s_append(out, "@LCL\nD=M\n@");
    s_append(out, index_str);
    s_append(out, "\nA=D+A\nD=M\n");
    s_append(out, hack_asm_push);
    return 0;
  case MS_STATIC:
    String symbol = s_new();
    s_append(&symbol, p->_file_name);
    s_append(&symbol, ".");
    s_append(&symbol, index_str);

    s_append(out, "@");
    s_append(out, symbol.data);
    s_append(out, "\nD=M\n");
    s_append(out, hack_asm_push);

    s_destroy(&symbol);
    return 0;
  case MS_CONSTANT:
    s_append(out, "@");
    s_append(out, index_str);
    s_append(out, "\nD=A\n");
    s_append(out, hack_asm_push);
    return 0;
  case MS_THIS:
    s_append(out, "@THIS\nD=M\n@");
    s_append(out, index_str);
    s_append(out, "\nA=D+A\nD=M\n");
    s_append(out, hack_asm_push);
    return 0;
  case MS_THAT:
    s_append(out, "@THAT\nD=M\n@");
    s_append(out, index_str);
    s_append(out, "\nA=D+A\nD=M\n");
    s_append(out, hack_asm_push);
    return 0;
  case MS_POINTER:
    if (cmd.index != 0 && cmd.index != 1) {
      return error_set(
          "error invalid index for pointer segment: expect 0 or 1, got %zu",
          cmd.index);
    }

    if (cmd.index == 0) {
      s_append(out, "@THIS\nD=M\n");
    } else {
      s_append(out, "@THAT\nD=M\n");
    }
    s_append(out, hack_asm_push);
    return 0;
  case MS_TEMP:
    char addr_str[SIZE_T_STR_MAX_DIGITS];
    res = to_str(TEMP_ADDR + cmd.index, sizeof(addr_str), addr_str);
    if (res < 0) {
      return -1;
    }

    s_append(out, "@");
    s_append(out, addr_str);
    s_append(out, "\nD=M\n");
    s_append(out, hack_asm_push);
    return 0;
  }

  return -1;
}

int parser_parse(Parser *p, const TokenList *tokens, String *out_hack) {
  s_clear(out_hack);
  if (tokens->count == 0) {
    return 0;
  }

  p->cur = 0;
  p->tokens = tokens;
  if (p->tokens->items[p->cur].kind == TK_PUSH) {
    p->cur++;
    CommandPush cmd = {0};
    int res = parser_read_segment(p, &cmd.segment);
    if (res < 0)
      return -1;

    res = parser_read_number(p, &cmd.index);
    if (res < 0)
      return -1;

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    res = command_push_translate(p, cmd, out_hack);
    if (res < 0) {
      return -1;
    }

    return 0;
  }

  if (p->tokens->items[p->cur].kind == TK_ADD) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    // pop the top two value of the stack, add them together and put the result
    // in the D register.
    const char *hack_assembly_add =
        "@SP\nA=M\nD=M\n@SP\nM=M-1\n@SP\nA=M\nD=D+M\n@SP\nM=M-1\n";
    s_set(out_hack, hack_assembly_add, strlen(hack_assembly_add));
    s_append(out_hack, hack_asm_push);
    return 0;
  }

  return error_set("unsupported command (token kind %d)",
                   p->tokens->items[0].kind);
}
int main(void) {
  int status = EXIT_SUCCESS;

  const char *lines[] = {" push static 4", "push constant 91", "add"};
  // Allocated memory when processing a single line of source VM code, so it
  // can be reseted afterward.
  Arena scratch = {0};
  Lexer lexer = {0};
  Parser p;
  parser_init(&p, "Foo");
  String hack = s_new();
  lexer_init(&lexer);

  for (size_t i = 0; i < sizeof(lines) / sizeof(*lines); i++) {
    TokenList tokens = {0};
    const char *line = lines[i];
    int res = lexer_tokenize(&lexer, &scratch, strlen(line), line, &tokens);
    if (res < 0) {
      fputs(error_get(), stderr);
      status = EXIT_FAILURE;
      break;
    }

    if (tokens.count == 0)
      continue;

    // puts(line);
    // for (size_t j = 0; j < tokens.count; j++) {
    //   Token item = tokens.items[j];
    //   printf("%d", item.kind);
    //   if (item.val_len > 0) {
    //     putchar(' ');
    //     fwrite(item.val, sizeof(char), item.val_len, stdout);
    //   }
    //   putchar('\n');
    // }

    res = parser_parse(&p, &tokens, &hack);
    if (res < 0) {
      fputs(error_get(), stderr);
      status = EXIT_FAILURE;
      break;
    }

    fputs(hack.data, stdout);

    arena_reset(&scratch);
  }

  s_destroy(&hack);
  lexer_deinit(&lexer);
  arena_free(&scratch);

  return status;
}