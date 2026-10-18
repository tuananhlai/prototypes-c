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

// TODO: think about a better error handling mechanism.
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

static Token token_create(TokenKind kind) { return (Token){.kind = kind}; }

static Token token_create3(TokenKind kind, size_t val_len,
                           const char val[val_len]) {
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

static const Token *tm_get(TokenMapEntry *tm, const char *keyword) {
  TokenMapEntry *retval = shgetp_null(tm, keyword);
  if (retval == nullptr) {
    return nullptr;
  }
  return &retval->value;
}

static void tm_destroy(TokenMapEntry *tm) { shfree(tm); }

typedef struct {
  Token *items;
  size_t count;
  size_t capacity;
} TokenList;

typedef struct {
  TokenMapEntry *builtin_tokens;
} Lexer;

static void lexer_init(Lexer *lexer) { lexer->builtin_tokens = tm_create(); }

static void lexer_deinit(Lexer *lexer) { tm_destroy(lexer->builtin_tokens); }

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

static int lexer_tokenize(Lexer *lexer, Arena *arena, size_t vm_line_len,
                          const char vm_line[vm_line_len],
                          TokenList *out_tokens) {
  size_t i = 0;
  size_t len = 0;

  while (i < vm_line_len) {
    while (i < vm_line_len && isspace(vm_line[i]))
      i++;
    while (i + len < vm_line_len && !isspace(vm_line[i + len]))
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
    arena_da_append(arena, out_tokens, token);

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

static void parser_init(Parser *p, const char *file_name) {
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

#define TEMP_ADDR 5

static int parser_translate_push_cmd(Parser *p, Arena *arena, CommandPush cmd,
                                     char **out) {
  switch (cmd.segment) {
  case MS_ARGUMENT:
    *out = arena_sprintf(arena, "@ARG\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  case MS_LOCAL:
    *out = arena_sprintf(arena, "@LCL\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  case MS_STATIC:
    *out = arena_sprintf(arena, "@%s.%zu\nD=M\n%s", p->_file_name, cmd.index,
                         hack_asm_push);
    return 0;
  case MS_CONSTANT:
    *out = arena_sprintf(arena, "@%zu\nD=A\n%s", cmd.index, hack_asm_push);
    return 0;
  case MS_THIS:
    *out = arena_sprintf(arena, "@THIS\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  case MS_THAT:
    *out = arena_sprintf(arena, "@THAT\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  case MS_POINTER:
    if (cmd.index != 0 && cmd.index != 1) {
      return error_set(
          "error invalid index for pointer segment: expect 0 or 1, got %zu",
          cmd.index);
    }

    const char *copy_this_reg_data = "@THIS\nD=M\n";
    const char *copy_that_reg_data = "@THAT\nD=M\n";
    const char *copy_reg_data =
        cmd.index == 0 ? copy_this_reg_data : copy_that_reg_data;
    *out = arena_sprintf(arena, "%s%s", copy_reg_data, hack_asm_push);
    return 0;
  case MS_TEMP:
    *out = arena_sprintf(arena, "@%zu\nD=M\n%s", TEMP_ADDR + cmd.index,
                         hack_asm_push);
    return 0;
  }

  fputs("unknown state reached", stderr);
  exit(-1);
}

typedef struct {
  MemorySegment segment;
  size_t index;
} CommandPop;

static const char *hack_asm_pop = "@SP\nAM=M-1\nD=M\n";

static int parser_translate_pop_cmd(Parser *p, Arena *arena, CommandPop cmd,
                                    char **out_hack) {

  switch (cmd.segment) {
  case MS_LOCAL:
    *out_hack = arena_sprintf(
        arena, "@%zu\nD=A\n@LCL\nD=D+M\n@R13\nM=D\n%s@R13\nA=M\nM=D\n",
        cmd.index, hack_asm_pop);
    return 0;
  default:
    return error_set("unsupported segment: %d", cmd.segment);
  }
}

static int parser_parse(Parser *p, Arena *arena, const TokenList *tokens,
                        char **out_hack) {
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

    res = parser_translate_push_cmd(p, arena, cmd, out_hack);
    if (res < 0) {
      return -1;
    }

    return 0;
  }

  if (p->tokens->items[p->cur].kind == TK_POP) {
    p->cur++;

    CommandPop cmd = {0};
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

    res = parser_translate_pop_cmd(p, arena, cmd, out_hack);
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
    *out_hack = arena_sprintf(arena, "%s%s", hack_assembly_add, hack_asm_push);
    return 0;
  }

  return error_set("unsupported command (token kind %d)",
                   p->tokens->items[0].kind);
}

/**
 * @brief Read a line from a file stream into a String.
 * @return EOF (-1) if we reached EOF and no data were read. 0 otherwise.
 */
static int readline(FILE *fp, String *s) {
  s_clear(s);

  int ch;
  while (true) {
    ch = fgetc(fp);
    if (ch == '\n' || ch == EOF) {
      break;
    }
    s_appendc(s, ch);
  }

  if (s->len == 0 && ch == EOF)
    return EOF;

  return 0;
}

/**
 * Translate the given VM language stream into Hack Assembly and write
 * the output to the given destination.
 */
int translate(const char *in_file_name, FILE *in, FILE *out) {
  Arena scratch = {0};
  Lexer lexer = {0};
  lexer_init(&lexer);
  Parser p;
  parser_init(&p, in_file_name);
  String line = s_new();
  int retval = -1;

  while (true) {
    char *hack = nullptr;
    int res = readline(in, &line);
    if (res == EOF)
      break;

    TokenList tokens = {0};
    res = lexer_tokenize(&lexer, &scratch, line.len, line.data, &tokens);
    if (res < 0) {
      fputs(error_get(), stderr);
      goto cleanup;
    }

    if (tokens.count == 0)
      continue;

    res = parser_parse(&p, &scratch, &tokens, &hack);
    if (res < 0) {
      fputs(error_get(), stderr);
      goto cleanup;
    }

    if (fputs(hack, out) == EOF && ferror(out)) {
      perror("cannot write to destination");
      goto cleanup;
    }

    arena_reset(&scratch);
  }
  retval = 0;

cleanup:
  s_destroy(&line);
  lexer_deinit(&lexer);
  arena_free(&scratch);
  return retval;
}
