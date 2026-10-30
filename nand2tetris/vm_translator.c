#include <ctype.h>
#include <errno.h>
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
  TK_FUNCTION,
  TK_RETURN,
  TK_CALL,
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
  shput(retval, "function", token_create(TK_FUNCTION));
  shput(retval, "return", token_create(TK_RETURN));
  shput(retval, "call", token_create(TK_CALL));
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
  // Allocate memory for the null-terminated map key. Free it immediately after
  // usage.
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
        return error_set("invalid number '%.*s'", (int)n, word);
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

typedef enum {
  CMD_PUSH,
  CMD_POP,
  CMD_ADD,
  CMD_SUB,
  CMD_AND,
  CMD_OR,
  CMD_NEG,
  CMD_NOT,
  CMD_EQ,
  CMD_GT,
  CMD_LT,
  CMD_LBL,
  CMD_GOTO,
  CMD_IFGOTO,
  CMD_FUNC,
  CMD_CALL,
  CMD_RET,
} CommandKind;

typedef struct {
  MemorySegment segment;
  size_t index;
} CommandPushArgs;

typedef struct {
  MemorySegment segment;
  size_t index;
} CommandPopArgs;

typedef struct {
  const char *symbol;
  size_t symbol_len;
} CommandLblArgs;

typedef struct {
  const char *symbol;
  size_t symbol_len;
} CommandGotoArgs;

typedef struct {
  const char *symbol;
  size_t symbol_len;
} CommandIfgotoArgs;

typedef struct {
  const char *fn_name;
  size_t fn_name_len;
  size_t n_vars;
} CommandFuncArgs;

typedef struct {
  const char *fn_name;
  size_t fn_name_len;
  size_t n_args;
} CommandCallArgs;

typedef struct {
  CommandKind kind;
  union {
    CommandPushArgs push;
    CommandPopArgs pop;
    CommandLblArgs lbl;
    CommandGotoArgs gotoz;
    CommandIfgotoArgs ifgoto;
    CommandFuncArgs func;
    CommandCallArgs call;
  } args;
} Command;

Command command_push_new(MemorySegment segment, size_t index) {
  return (Command){
      .kind = CMD_PUSH,
      .args = {.push = {segment, index}},
  };
}
Command command_pop_new(MemorySegment segment, size_t index) {
  return (Command){
      .kind = CMD_POP,
      .args = {.pop = {segment, index}},
  };
}
Command command_add_new(void) { return (Command){.kind = CMD_ADD}; }
Command command_sub_new(void) { return (Command){.kind = CMD_SUB}; }
Command command_and_new(void) { return (Command){.kind = CMD_AND}; }
Command command_or_new(void) { return (Command){.kind = CMD_OR}; }
Command command_neg_new(void) { return (Command){.kind = CMD_NEG}; }
Command command_not_new(void) { return (Command){.kind = CMD_NOT}; }
Command command_eq_new(void) { return (Command){.kind = CMD_EQ}; }
Command command_gt_new(void) { return (Command){.kind = CMD_GT}; }
Command command_lt_new(void) { return (Command){.kind = CMD_LT}; }
Command command_lbl_new(size_t symbol_len, const char symbol[symbol_len]) {
  return (Command){
      .kind = CMD_LBL,
      .args = {.lbl = {symbol, symbol_len}},
  };
}
Command command_goto_new(size_t symbol_len, const char symbol[symbol_len]) {
  return (Command){
      .kind = CMD_GOTO,
      .args = {.gotoz = {symbol, symbol_len}},
  };
}
Command command_ifgoto_new(size_t symbol_len, const char symbol[symbol_len]) {
  return (Command){
      .kind = CMD_IFGOTO,
      .args = {.ifgoto = {symbol, symbol_len}},
  };
}
Command command_func_new(size_t symbol_len, const char symbol[symbol_len],
                         size_t n_vars) {
  return (Command){
      .kind = CMD_FUNC,
      .args = {.func = {symbol, symbol_len, n_vars}},
  };
}
Command command_call_new(size_t symbol_len, const char symbol[symbol_len],
                         size_t n_args) {
  return (Command){
      .kind = CMD_CALL,
      .args = {.call = {symbol, symbol_len, n_args}},
  };
}
Command command_ret_new(void) { return (Command){.kind = CMD_RET}; }

static void parser_init(Parser *p) {
  p->cur = 0;
  p->tokens = nullptr;
}

// Hack Assembly code which pushes the value stored inside the D register
// into the stack.
static const char *hack_asm_push = "@SP\nAM=M+1\nA=A-1\nM=D\n";

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

static int parser_read_symbol(Parser *p, size_t *out_n,
                              const char **out_symbol) {
  if (p->cur >= p->tokens->count)
    return error_set("expected symbol, got end of line");
  if (p->tokens->items[p->cur].kind != TK_SYMBOL) {
    return error_set("expect symbol at token %zu", p->cur);
  }

  Token cur = p->tokens->items[p->cur];
  *out_n = cur.val_len;
  *out_symbol = cur.val;
  p->cur++;
  return 0;
}

#define TEMP_ADDR 5

static const char *hack_asm_pop = "@SP\nAM=M-1\nD=M\n";

#define FN_FRAME_SIZE 5

/**
 * Parse the list of tokens into a Hack Assembly string. Its memory will be
 * allocated from the given arena allocator.
 */
static int parser_parse_line(Parser *p, const TokenList *tokens,
                             Command *out_cmd) {
  if (tokens->count == 0) {
    return 0;
  }

  p->cur = 0;
  p->tokens = tokens;
  TokenKind cur_token_kind = p->tokens->items[p->cur].kind;
  if (cur_token_kind == TK_PUSH) {
    p->cur++;
    MemorySegment segment;
    int res = parser_read_segment(p, &segment);
    if (res < 0)
      return -1;

    size_t index = 0;
    res = parser_read_number(p, &index);
    if (res < 0)
      return -1;

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    *out_cmd = command_push_new(segment, index);
    return 0;
  }

  if (cur_token_kind == TK_POP) {
    p->cur++;

    MemorySegment segment;
    int res = parser_read_segment(p, &segment);
    if (res < 0)
      return -1;

    size_t index = 0;
    res = parser_read_number(p, &index);
    if (res < 0)
      return -1;

    res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    *out_cmd = command_pop_new(segment, index);
    return 0;
  }

  if (cur_token_kind == TK_ADD || cur_token_kind == TK_SUB ||
      cur_token_kind == TK_AND || cur_token_kind == TK_OR) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    if (cur_token_kind == TK_ADD)
      *out_cmd = command_add_new();
    else if (cur_token_kind == TK_SUB)
      *out_cmd = command_sub_new();
    else if (cur_token_kind == TK_AND)
      *out_cmd = command_and_new();
    else if (cur_token_kind == TK_OR)
      *out_cmd = command_or_new();
    else
      return error_set("process unary expression: unknown token kind %d",
                       cur_token_kind);

    return 0;
  }

  if (cur_token_kind == TK_NEG || cur_token_kind == TK_NOT) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    if (cur_token_kind == TK_NEG)
      *out_cmd = command_neg_new();
    else if (cur_token_kind == TK_NOT)
      *out_cmd = command_not_new();
    else
      return error_set("process unary expression: unknown token kind %d",
                       cur_token_kind);

    return 0;
  }

  if (cur_token_kind == TK_EQ || cur_token_kind == TK_GT ||
      cur_token_kind == TK_LT) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0) {
      return -1;
    }

    if (cur_token_kind == TK_EQ)
      *out_cmd = command_eq_new();
    else if (cur_token_kind == TK_GT)
      *out_cmd = command_gt_new();
    else if (cur_token_kind == TK_LT)
      *out_cmd = command_lt_new();
    else {
      return error_set("process comparison expression: unknown token kind %d",
                       cur_token_kind);
    }

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

    *out_cmd = command_lbl_new(n, symbol);
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

    *out_cmd = command_goto_new(n, symbol);
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

    *out_cmd = command_ifgoto_new(n, symbol);
    return 0;
  }

  if (cur_token_kind == TK_CALL) {
    p->cur++;

    size_t n = 0;
    const char *symbol = nullptr;
    int res = parser_read_symbol(p, &n, &symbol);
    if (res < 0)
      return -1;

    size_t n_args = 0;
    res = parser_read_number(p, &n_args);
    if (res < 0)
      return -1;

    res = parser_read_eol(p);
    if (res < 0)
      return -1;

    *out_cmd = command_call_new(n, symbol, n_args);
    return 0;
  }

  if (cur_token_kind == TK_FUNCTION) {
    p->cur++;

    size_t n = 0;
    const char *symbol = nullptr;
    int res = parser_read_symbol(p, &n, &symbol);
    if (res < 0)
      return -1;

    size_t n_args = 0;
    res = parser_read_number(p, &n_args);
    if (res < 0)
      return -1;

    res = parser_read_eol(p);
    if (res < 0)
      return -1;

    *out_cmd = command_func_new(n, symbol, n_args);
    return 0;
  }

  if (cur_token_kind == TK_RETURN) {
    p->cur++;

    int res = parser_read_eol(p);
    if (res < 0)
      return -1;

    *out_cmd = command_ret_new();
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

typedef struct {
  Lexer lexer;
  Parser parser;
  // Reset after parsing each line of VM code.
  Arena line_arena;
  // Destroy once when Translator is destroyed.
  Arena perm_arena;
  const char *_file_name;
  // Name of the function for which we're currently processing commands.
  char *_cur_func_name;
  // Counter used to generate unique internal labels.
  size_t _label_idx;
} Translator;

void translator_init(Translator *t, const char *file_name) {
  t->lexer = (Lexer){0};
  lexer_init(&t->lexer);
  t->parser = (Parser){0};
  parser_init(&t->parser);
  t->line_arena = (Arena){0};
  t->_file_name = file_name;
  t->_cur_func_name = nullptr;
  t->_label_idx = 0;
}

void translator_deinit(Translator *t) {
  free(t->_cur_func_name);
  lexer_deinit(&t->lexer);
  arena_free(&t->line_arena);
}

static const char *translator_gen_label(Translator *t) {
  return arena_sprintf(&t->line_arena, "L%zu", t->_label_idx++);
}

static int translator_translate_push(Translator *t, CommandPushArgs args,
                                     char **out) {
  Arena *arena = &t->line_arena;

  switch (args.segment) {
  case MS_ARGUMENT: {
    *out = arena_sprintf(arena, "@ARG\nD=M\n@%zu\nA=D+A\nD=M\n%s", args.index,
                         hack_asm_push);
    return 0;
  }
  case MS_LOCAL: {
    *out = arena_sprintf(arena, "@LCL\nD=M\n@%zu\nA=D+A\nD=M\n%s", args.index,
                         hack_asm_push);
    return 0;
  }
  case MS_STATIC: {
    *out = arena_sprintf(arena, "@%s.%zu\nD=M\n%s", t->_file_name, args.index,
                         hack_asm_push);
    return 0;
  }
  case MS_CONSTANT: {
    *out = arena_sprintf(arena, "@%zu\nD=A\n%s", args.index, hack_asm_push);
    return 0;
  }
  case MS_THIS: {
    *out = arena_sprintf(arena, "@THIS\nD=M\n@%zu\nA=D+A\nD=M\n%s", args.index,
                         hack_asm_push);
    return 0;
  }
  case MS_THAT: {
    *out = arena_sprintf(arena, "@THAT\nD=M\n@%zu\nA=D+A\nD=M\n%s", args.index,
                         hack_asm_push);
    return 0;
  }
  case MS_POINTER: {
    if (args.index != 0 && args.index != 1) {
      return error_set(
          "error invalid index for pointer segment: expect 0 or 1, got %zu",
          args.index);
    }

    const char *src_addr = args.index == 0 ? "THIS" : "THAT";
    *out = arena_sprintf(arena, "@%s\nD=M\n%s", src_addr, hack_asm_push);
    return 0;
  }
  case MS_TEMP: {
    *out = arena_sprintf(arena, "@%zu\nD=M\n%s", TEMP_ADDR + args.index,
                         hack_asm_push);
    return 0;
  }
  }
  return error_set("unsupported segment: %d", args.segment);
}

static int translator_translate_pop(Translator *t, CommandPopArgs args,
                                    char **out) {
  Arena *arena = &t->line_arena;

  switch (args.segment) {
  case MS_LOCAL:
  case MS_ARGUMENT:
  case MS_THIS:
  case MS_THAT: {
    const char *dest_addr;
    if (args.segment == MS_LOCAL)
      dest_addr = "LCL";
    else if (args.segment == MS_ARGUMENT)
      dest_addr = "ARG";
    else if (args.segment == MS_THIS)
      dest_addr = "THIS";
    else if (args.segment == MS_THAT)
      dest_addr = "THAT";
    else
      return -1;

    *out = arena_sprintf(arena,
                         "@%zu\nD=A\n@%s\nD=D+M\n@R13\nM=D\n%s@R13\nA=M\nM=D\n",
                         args.index, dest_addr, hack_asm_pop);
    return 0;
  }
  case MS_TEMP: {
    *out = arena_sprintf(arena, "%s@%zu\nM=D\n", hack_asm_pop,
                         TEMP_ADDR + args.index);
    return 0;
  }
  case MS_POINTER: {
    const char *dest_addr = args.index == 0 ? "THIS" : "THAT";
    *out = arena_sprintf(arena, "%s@%s\nM=D\n", hack_asm_pop, dest_addr);
    return 0;
  }
  case MS_STATIC: {
    *out = arena_sprintf(arena, "%s@%s.%zu\nM=D\n", hack_asm_pop, t->_file_name,
                         args.index);
    return 0;
  }
  default: {
    return error_set("unsupported segment: %d", args.segment);
  }
  }
}

static const char *translator_next_retaddr(Translator *t, size_t n,
                                           const char fn_name[n]) {
  return arena_sprintf(&t->line_arena, "%.*s$ret.%zu", (int)n, fn_name,
                       t->_label_idx++);
}

static const char *translator_get_label(Translator *t, size_t n,
                                        const char label[n]) {
  if (!t->_cur_func_name) {
    return arena_sprintf(&t->line_arena, "%.*s", (int)n, label);
  }

  return arena_sprintf(&t->line_arena, "%s$%.*s", t->_cur_func_name, (int)n,
                       label);
}

static int translator_translate_cmd(Translator *t, Command cmd, char **out) {
  Arena *arena = &t->line_arena;

  switch (cmd.kind) {
  case CMD_PUSH:
    return translator_translate_push(t, cmd.args.push, out);
  case CMD_POP:
    return translator_translate_pop(t, cmd.args.pop, out);
  case CMD_ADD:
  case CMD_SUB:
  case CMD_AND:
  case CMD_OR: {
    const char *set_data_reg_cmd;
    if (cmd.kind == CMD_ADD)
      set_data_reg_cmd = "D=D+M";
    else if (cmd.kind == CMD_SUB)
      set_data_reg_cmd = "D=M-D";
    else if (cmd.kind == CMD_AND)
      set_data_reg_cmd = "D=D&M";
    else
      set_data_reg_cmd = "D=D|M";

    // pop the top two value of the stack, add them together and put the result
    // in the D register.
    const char *hack_assembly_calc = arena_sprintf(
        arena, "@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\n%s\n", set_data_reg_cmd);
    *out = arena_sprintf(arena, "%s%s", hack_assembly_calc, hack_asm_push);
    return 0;
  }
  case CMD_NEG:
  case CMD_NOT: {
    const char *set_data_reg_cmd;
    if (cmd.kind == CMD_NEG)
      set_data_reg_cmd = "D=-M";
    else
      set_data_reg_cmd = "D=!M";

    const char *hack_assembly_calc =
        arena_sprintf(arena, "@SP\nAM=M-1\n%s\n", set_data_reg_cmd);
    *out = arena_sprintf(arena, "%s%s", hack_assembly_calc, hack_asm_push);
    return 0;
  }
  case CMD_EQ:
  case CMD_GT:
  case CMD_LT: {
    const char *push_true_label = translator_gen_label(t);
    const char *push_false_label = translator_gen_label(t);

    const char *conditional_jmp;
    if (cmd.kind == CMD_EQ)
      conditional_jmp = "JEQ";
    else if (cmd.kind == CMD_GT)
      conditional_jmp = "JGT";
    else
      conditional_jmp = "JLT";

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
    *out = arena_sprintf(arena, "%s%s", hack_assembly_compare, hack_asm_push);
    return 0;
  }
  case CMD_LBL: {
    CommandLblArgs args = cmd.args.lbl;
    *out = arena_sprintf(arena, "(%s)\n",
                         translator_get_label(t, args.symbol_len, args.symbol));
    return 0;
  }
  case CMD_GOTO: {
    CommandGotoArgs args = cmd.args.gotoz;
    *out = arena_sprintf(arena, "@%s\n0;JMP\n",
                         translator_get_label(t, args.symbol_len, args.symbol));
    return 0;
  }
  case CMD_IFGOTO: {
    CommandIfgotoArgs args = cmd.args.ifgoto;
    *out = arena_sprintf(arena,
                         "@SP\n"
                         "AM=M-1\n"
                         "D=M\n"
                         "@%s\n"
                         "D;JNE\n",
                         translator_get_label(t, args.symbol_len, args.symbol));
    return 0;
  }
  case CMD_CALL: {
    CommandCallArgs args = cmd.args.call;

    const char *return_addr =
        translator_next_retaddr(t, args.fn_name_len, args.fn_name);

    *out =
        arena_sprintf(arena,
                      "@%s\n" // push return address, current value of LCL,
                              // ARGS, THIS, THAT to stack
                      "D=A\n"
                      "@SP\n"
                      "AM=M+1\n"
                      "A=A-1\n"
                      "M=D\n"
                      "@LCL\n"
                      "D=M\n"
                      "@SP\n"
                      "AM=M+1\n"
                      "A=A-1\n"
                      "M=D\n"
                      "@ARG\n"
                      "D=M\n"
                      "@SP\n"
                      "AM=M+1\n"
                      "A=A-1\n"
                      "M=D\n"
                      "@THIS\n"
                      "D=M\n"
                      "@SP\n"
                      "AM=M+1\n"
                      "A=A-1\n"
                      "M=D\n"
                      "@THAT\n"
                      "D=M\n"
                      "@SP\n"
                      "AM=M+1\n"
                      "A=A-1\n"
                      "M=D\n"
                      "@%zu\n" // *ARG = *SP - frame_size - number of arguments
                      "D=A\n"
                      "@SP\n"
                      "D=M-D\n"
                      "@ARG\n"
                      "M=D\n"
                      "@SP\n" // *LCL = *SP
                      "D=M\n"
                      "@LCL\n"
                      "M=D\n"
                      "@%.*s\n" // jump to the target function
                      "0;JMP\n"
                      "(%s)",
                      return_addr, FN_FRAME_SIZE + args.n_args,
                      (int)args.fn_name_len, args.fn_name, return_addr);
    return 0;
  }
  case CMD_FUNC:
    CommandFuncArgs args = cmd.args.func;
    if (!t->_cur_func_name || strlen(t->_cur_func_name) < args.fn_name_len) {
      t->_cur_func_name = realloc(t->_cur_func_name, args.fn_name_len + 1);
    }
    t->_cur_func_name[args.fn_name_len] = '\0';
    memcpy(t->_cur_func_name, args.fn_name, args.fn_name_len);

    const char *loop_label = translator_gen_label(t);
    const char *loop_end_label = translator_gen_label(t);
    // TODO: incorrect return value pushing + go to return address logic.
    *out = arena_sprintf(
        &t->line_arena,
        "(%.*s)\n" // fn name
        "@%zu\n"
        "D=A\n"
        "@R13\n"
        "M=D-1\n" // remove 1 so that the loop runs exactly `n_vars` times.
        "(%s)\n"  // loop start
        "@R13\n"
        "D=M\n"
        "@%s\n" // loop end
        "D;JEQ\n"
        "@SP\n"
        "AM=M+1\n"
        "A=A-1\n"
        "M=0\n"
        "@R13\n"
        "M=M-1\n"
        "@%s\n" // loop start
        "0;JMP\n"
        "(%s)\n", // loop end
        (int)args.fn_name_len, args.fn_name, args.n_vars, loop_label,
        loop_end_label, loop_label, loop_end_label);
    return 0;
  case CMD_RET:
    *out = "@ARG\n"
           "D=M\n"
           "@R15\n"
           "M=D\n"
           "@SP\n"
           "AM=M-1\n"
           "D=M\n"
           "@R13\n" // store result in R13
           "M=D\n"
           "@LCL\n" // *SP = *LCL
           "D=M\n"
           "@SP\n"
           "AM=D-1\n"
           "D=M\n"
           "@THAT\n"
           "M=D\n"
           "@SP\n"
           "AM=M-1\n"
           "D=M\n"
           "@THIS\n"
           "M=D\n"
           "@SP\n"
           "AM=M-1\n"
           "D=M\n"
           "@ARG\n"
           "M=D\n"
           "@SP\n"
           "AM=M-1\n"
           "D=M\n"
           "@LCL\n"
           "M=D\n"
           "@SP\n"
           "AM=M-1\n"
           "D=M\n"
           "@R14\n"
           "M=D\n"
           "@R15\n" // *SP = old *ARGS
           "D=M\n"
           "@SP\n"
           "M=D\n"
           "@R13\n" // push result from R13 to top of the stack
           "D=M\n"
           "@SP\n"
           "AM=M+1\n"
           "A=A-1\n"
           "M=D\n"
           "@R14\n" // jump to return address stored in R14
           "A=M\n"
           "0;JMP\n";
    return 0;
  }

  return error_set("unsupported command kind %d", cmd.kind);
}

int translator_translate(Translator *t, FILE *in, FILE *out) {
  arena_reset(&t->line_arena);

  while (true) {
    char *line = nullptr;
    size_t line_len = 0;
    int res = readline(in, &t->line_arena, &line, &line_len);
    if (res == EOF)
      break;
    if (line_len == 0)
      continue;

    TokenList tokens = {0};
    res = lexer_tokenize(&t->lexer, &t->line_arena, line_len, line, &tokens);
    if (res < 0) {
      return -1;
    }
    if (tokens.count == 0)
      continue;

    Command cmd = {0};
    res = parser_parse_line(&t->parser, &tokens, &cmd);
    if (res < 0) {
      return -1;
    }

    char *hack = nullptr;
    res = translator_translate_cmd(t, cmd, &hack);
    if (res < 0) {
      return -1;
    }
    if (fputs(hack, out) == EOF && ferror(out)) {
      return error_set("cannot write to destination: %s", strerror(errno));
    }

    arena_reset(&t->line_arena);
  }

  return 0;
}

/**
 * Translate the given VM language stream into Hack Assembly and write
 * the output to the given destination.
 */
int translate(const char *in_file_name, FILE *in, FILE *out) {
  int res = 0;

  Translator code_translator = {0};
  translator_init(&code_translator, in_file_name);
  res = translator_translate(&code_translator, in, out);
  translator_deinit(&code_translator);

  return res;
}
