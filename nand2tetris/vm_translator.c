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
  TK_NEG,
  TK_EQ,
  TK_GT,
  TK_LT,
  TK_AND,
  TK_OR,
  TK_NOT,
  TK_CONSTANT,
  TK_LOCAL,
  TK_ARGUMENT,
  TK_THIS,
  TK_THAT,
  TK_TEMP,
  TK_STATIC,
  TK_POINTER,
  TK_NUMBER,
  TK_LABEL,
  TK_IFGOTO,
  TK_GOTO,
  TK_SYMBOL,
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
  shput(retval, "neg", token_create(TK_NEG));
  shput(retval, "eq", token_create(TK_EQ));
  shput(retval, "gt", token_create(TK_GT));
  shput(retval, "lt", token_create(TK_LT));
  shput(retval, "and", token_create(TK_AND));
  shput(retval, "or", token_create(TK_OR));
  shput(retval, "not", token_create(TK_NOT));
  shput(retval, "constant", token_create(TK_CONSTANT));
  shput(retval, "local", token_create(TK_LOCAL));
  shput(retval, "argument", token_create(TK_ARGUMENT));
  shput(retval, "this", token_create(TK_THIS));
  shput(retval, "that", token_create(TK_THAT));
  shput(retval, "static", token_create(TK_STATIC));
  shput(retval, "temp", token_create(TK_TEMP));
  shput(retval, "pointer", token_create(TK_POINTER));
  shput(retval, "label", token_create(TK_LABEL));
  shput(retval, "if-goto", token_create(TK_IFGOTO));
  shput(retval, "goto", token_create(TK_GOTO));
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

static const Token *lexer_get_builtin_token(Lexer *lexer, Arena *arena,
                                            size_t n, const char word[n]) {
  // Allocate memory for the null-terminated map key. Free it immediately after usage.
  Arena_Mark mark = arena_snapshot(arena);
  char *key = arena_alloc(arena, n + 1);
  memcpy(key, word, n);
  key[n] = '\0';
  const Token *builtin_token = tm_get(lexer->builtin_tokens, key);
  arena_rewind(arena, mark);
  return builtin_token;
}

static int resolve_word(Lexer *lexer, Arena *arena, size_t n,
                        const char word[n], Token *out_token) {
  if (n == 0) {
    return error_set("empty word");
  }

  const Token *builtin_token = lexer_get_builtin_token(lexer, arena, n, word);
  if (builtin_token) {
    *out_token = *builtin_token;
    return 0;
  }

  // TODO: do I really need to copy word?
  if (isdigit(word[0])) {
    for (size_t i = 1; i < n; i++) {
      if (!isdigit(word[i]))
        return error_set("invalid number '%s'", word);
    }
    *out_token = token_create3(TK_NUMBER, n, word);
    return 0;
  }

  // TODO: what does a valid symbol look like?
  *out_token = token_create3(TK_SYMBOL, n, word);
  return 0;
}

/**
 * Convert the given VM code into a list of tokens. Memories for both the list
 * and the elements will be allocated from the given arena allocator.
 */
static int lexer_tokenize(Lexer *lexer, Arena *arena, size_t vm_line_len,
                          const char vm_line[vm_line_len],
                          TokenList *out_tokens) {
  size_t i = 0;
  size_t len = 0;

  while (i < vm_line_len) {
    while (i < vm_line_len && isspace(vm_line[i]))
      i++;

    // Ignore the rest of the line if '//' is encountered.
    if (i + 1 < vm_line_len && vm_line[i] == '/' && vm_line[i + 1] == '/') {
      break;
    }

    while (i + len < vm_line_len && !isspace(vm_line[i + len]))
      len++;
    if (len == 0) {
      break;
    }

    Token token = {0};
    int res = resolve_word(lexer, arena, len, vm_line + i, &token);
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
  size_t _label_idx;
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
  p->_label_idx = 0;
}

// Hack Assembly code which pushes the value stored inside the D register
// into the stack.
static const char *hack_asm_push = "@SP\nA=M\nM=D\n@SP\nM=M+1\n";

static int parser_read_segment(Parser *p, MemorySegment *out_ms) {
  if (p->cur >= p->tokens->count) {
    return error_set("expected memory segment, got end of line");
  }

  switch (p->tokens->items[p->cur].kind) {
  case TK_CONSTANT: {
    *out_ms = MS_CONSTANT;
    break;
  }
  case TK_LOCAL: {
    *out_ms = MS_LOCAL;
    break;
  }
  case TK_ARGUMENT: {
    *out_ms = MS_ARGUMENT;
    break;
  }
  case TK_THIS: {
    *out_ms = MS_THIS;
    break;
  }
  case TK_THAT: {
    *out_ms = MS_THAT;
    break;
  }
  case TK_TEMP: {
    *out_ms = MS_TEMP;
    break;
  }
  case TK_STATIC: {
    *out_ms = MS_STATIC;
    break;
  }
  case TK_POINTER: {
    *out_ms = MS_POINTER;
    break;
  }
  default: {
    return error_set("expected memory segment at token %zu", p->cur + 1);
  }
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

static int parser_read_symbol(Parser *p, size_t *n, const char **symbol) {
  if (p->cur >= p->tokens->count)
    return error_set("expected symbol, got end of line");
  if (p->tokens->items[p->cur].kind != TK_SYMBOL) {
    return error_set("expect symbol at token %zu", p->cur);
  }

  Token cur = p->tokens->items[p->cur];
  *n = cur.val_len;
  *symbol = cur.val;
  p->cur++;
  return 0;
}

#define TEMP_ADDR 5

static int parser_translate_push_cmd(Parser *p, Arena *arena, CommandPush cmd,
                                     char **out) {
  switch (cmd.segment) {
  case MS_ARGUMENT: {
    *out = arena_sprintf(arena, "@ARG\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  }
  case MS_LOCAL: {
    *out = arena_sprintf(arena, "@LCL\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  }
  case MS_STATIC: {
    *out = arena_sprintf(arena, "@%s.%zu\nD=M\n%s", p->_file_name, cmd.index,
                         hack_asm_push);
    return 0;
  }
  case MS_CONSTANT: {
    *out = arena_sprintf(arena, "@%zu\nD=A\n%s", cmd.index, hack_asm_push);
    return 0;
  }
  case MS_THIS: {
    *out = arena_sprintf(arena, "@THIS\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  }
  case MS_THAT: {
    *out = arena_sprintf(arena, "@THAT\nD=M\n@%zu\nA=D+A\nD=M\n%s", cmd.index,
                         hack_asm_push);
    return 0;
  }
  case MS_POINTER: {
    if (cmd.index != 0 && cmd.index != 1) {
      return error_set(
          "error invalid index for pointer segment: expect 0 or 1, got %zu",
          cmd.index);
    }

    const char *src_addr = cmd.index == 0 ? "THIS" : "THAT";
    *out = arena_sprintf(arena, "@%s\nD=M\n%s", src_addr, hack_asm_push);
    return 0;
  }
  case MS_TEMP: {
    *out = arena_sprintf(arena, "@%zu\nD=M\n%s", TEMP_ADDR + cmd.index,
                         hack_asm_push);
    return 0;
  }
  }
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
  case MS_ARGUMENT:
  case MS_THIS:
  case MS_THAT: {
    const char *dest_addr;
    if (cmd.segment == MS_LOCAL)
      dest_addr = "LCL";
    else if (cmd.segment == MS_ARGUMENT)
      dest_addr = "ARG";
    else if (cmd.segment == MS_THIS)
      dest_addr = "THIS";
    else if (cmd.segment == MS_THAT)
      dest_addr = "THAT";
    else
      return -1;

    *out_hack = arena_sprintf(
        arena, "@%zu\nD=A\n@%s\nD=D+M\n@R13\nM=D\n%s@R13\nA=M\nM=D\n",
        cmd.index, dest_addr, hack_asm_pop);
    return 0;
  }
  case MS_TEMP: {
    *out_hack = arena_sprintf(arena, "%s@%zu\nM=D\n", hack_asm_pop,
                              TEMP_ADDR + cmd.index);
    return 0;
  }
  case MS_POINTER: {
    const char *dest_addr = cmd.index == 0 ? "THIS" : "THAT";
    *out_hack = arena_sprintf(arena, "%s@%s\nM=D\n", hack_asm_pop, dest_addr);
    return 0;
  }
  case MS_STATIC: {
    *out_hack = arena_sprintf(arena, "%s@%s.%zu\nM=D\n", hack_asm_pop,
                              p->_file_name, cmd.index);
    return 0;
  }
  default: {
    return error_set("unsupported segment: %d", cmd.segment);
  }
  }
}

static const char *parser_gen_label(Parser *p, Arena *arena) {
  return arena_sprintf(arena, "L%zu", p->_label_idx++);
}

/**
 * Parse the list of tokens into a Hack Assembly string. Its memory will be
 * allocated from the given arena allocator.
 */
static int parser_parse(Parser *p, Arena *arena, const TokenList *tokens,
                        char **out_hack) {
  if (tokens->count == 0) {
    return 0;
  }

  p->cur = 0;
  p->tokens = tokens;
  TokenKind cur_token_kind = p->tokens->items[p->cur].kind;
  if (cur_token_kind == TK_PUSH) {
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

  if (cur_token_kind == TK_POP) {
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

  if (cur_token_kind == TK_ADD || cur_token_kind == TK_SUB ||
      cur_token_kind == TK_AND || cur_token_kind == TK_OR) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    const char *set_data_reg_cmd;
    if (cur_token_kind == TK_ADD)
      set_data_reg_cmd = "D=D+M";
    else if (cur_token_kind == TK_SUB)
      set_data_reg_cmd = "D=M-D";
    else if (cur_token_kind == TK_AND)
      set_data_reg_cmd = "D=D&M";
    else if (cur_token_kind == TK_OR)
      set_data_reg_cmd = "D=D|M";
    else
      return error_set("process unary expression: unknown token kind %d",
                       cur_token_kind);

    // pop the top two value of the stack, add them together and put the result
    // in the D register.
    const char *hack_assembly_calc = arena_sprintf(
        arena, "@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\n%s\n", set_data_reg_cmd);
    *out_hack = arena_sprintf(arena, "%s%s", hack_assembly_calc, hack_asm_push);
    return 0;
  }

  if (cur_token_kind == TK_NEG || cur_token_kind == TK_NOT) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    const char *set_data_reg_cmd;
    if (cur_token_kind == TK_NEG)
      set_data_reg_cmd = "D=-M";
    else if (cur_token_kind == TK_NOT)
      set_data_reg_cmd = "D=!M";
    else
      return error_set("process unary expression: unknown token kind %d",
                       cur_token_kind);

    const char *hack_assembly_calc =
        arena_sprintf(arena, "@SP\nAM=M-1\n%s\n", set_data_reg_cmd);
    *out_hack = arena_sprintf(arena, "%s%s", hack_assembly_calc, hack_asm_push);
    return 0;
  }

  if (cur_token_kind == TK_EQ || cur_token_kind == TK_GT ||
      cur_token_kind == TK_LT) {
    p->cur++;

    const char *push_true_label = parser_gen_label(p, arena);
    const char *push_false_label = parser_gen_label(p, arena);

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    const char *conditional_jmp;
    if (cur_token_kind == TK_EQ)
      conditional_jmp = "JEQ";
    else if (cur_token_kind == TK_GT)
      conditional_jmp = "JGT";
    else if (cur_token_kind == TK_LT)
      conditional_jmp = "JLT";
    else {
      return error_set("process comparison expression: unknown token kind %d",
                       cur_token_kind);
    }

    const char *hack_assembly_compare =
        arena_sprintf(arena,
                      "@SP\n"
                      "AM=M-1\n"
                      "D=M\n"
                      "@SP\n"
                      "AM=M-1\n"
                      "D=M-D\n"
                      "@%s\n"  // push true
                      "D;%s\n" // conditional jump command
                      "@%s\n"  // push false
                      "D=0;JMP\n"
                      "(%s)\n" // push true
                      "D=-1\n"
                      "(%s)\n", // push false
                      push_true_label, conditional_jmp, push_false_label,
                      push_true_label, push_false_label);
    *out_hack =
        arena_sprintf(arena, "%s%s", hack_assembly_compare, hack_asm_push);
    return 0;
  }

  if (cur_token_kind == TK_LABEL) {
    p->cur++;

    size_t n = 0;
    const char *symbol = nullptr;
    int res = parser_read_symbol(p, &n, &symbol);
    if (res < 0) {
      return -1;
    }

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    *out_hack = arena_sprintf(arena, "(%*s)\n", (int)n, symbol);
    return 0;
  }

  if (cur_token_kind == TK_GOTO) {
    p->cur++;

    size_t n = 0;
    const char *symbol = nullptr;
    int res = parser_read_symbol(p, &n, &symbol);
    if (res < 0) {
      return -1;
    }

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    *out_hack = arena_sprintf(arena, "@%*s\n0;JMP\n", (int)n, symbol);
    return 0;
  }

  if (cur_token_kind == TK_IFGOTO) {
    p->cur++;

    size_t n = 0;
    const char *symbol = nullptr;
    int res = parser_read_symbol(p, &n, &symbol);
    if (res < 0) {
      return -1;
    }

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    *out_hack = arena_sprintf(arena,
                              "@SP\n"
                              "AM=M-1\n"
                              "D=M\n"
                              "@%*s\n"
                              "D;JNE\n",
                              (int)n, symbol);
    return 0;
  }

  return error_set("unsupported command (token kind %d)",
                   p->tokens->items[0].kind);
}

/**
 * @brief Read a line from a file stream into `out`. Memories will be allocated
 * from the given arena allocator.
 * @return EOF (-1) if we reached EOF and no data were read. 0 otherwise.
 */
static int readline(FILE *fp, Arena *arena, char **out, size_t *out_len) {
  struct StringBuilder {
    char *items;
    size_t count;
    size_t capacity;
  };

  struct StringBuilder sb = {0};

  int ch;
  while (true) {
    ch = fgetc(fp);
    if (ch == '\n' || ch == EOF) {
      break;
    }
    arena_da_append(arena, &sb, ch);
  }

  if (sb.count == 0 && ch == EOF)
    return EOF;

  arena_da_append(arena, &sb, '\0');
  *out = sb.items;
  *out_len = sb.count - 1;
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
  int retval = -1;

  while (true) {
    char *line = nullptr;
    size_t line_len = 0;
    char *hack = nullptr;
    int res = readline(in, &scratch, &line, &line_len);
    if (res == EOF)
      break;
    if (line_len == 0)
      continue;

    TokenList tokens = {0};
    res = lexer_tokenize(&lexer, &scratch, line_len, line, &tokens);
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
  lexer_deinit(&lexer);
  arena_free(&scratch);
  return retval;
}
