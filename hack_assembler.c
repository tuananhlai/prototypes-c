#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynamic_string.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef enum {
  A_INSTRUCTION,
  C_INSTRUCTION,
  L_INSTRUCTION,
} InstructionType;

typedef struct {
  FILE* f;
  String cur_ins;
  String next_ins;

  String symbol;
  String dest;
  String comp;
  String jump;
} Parser;

int readline(FILE* fp, String* s);

Parser* parser_create(FILE* f);
bool parser_has_more_lines(Parser* p);
void parser_advance(Parser* p);
InstructionType parser_instruction_type(Parser* p);
const char* parser_symbol(Parser* p);
const char* parser_dest(Parser* p);
const char* parser_comp(Parser* p);
const char* parser_jump(Parser* p);
void parser_destroy(Parser* p);

bool is_digit_string(const char* s, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (!isdigit(s[i])) {
      return false;
    }
  }
  return true;
}

/**
 * A symbol can be any sequence of letters, digits, underscore (_), dot (.),
 * dollar sign ($), and colon (:) that does not begin with a digit.
 */
bool is_symbol_char(char c) {
  return isalnum(c) || c == '.' || c == '_' || c == '$' || c == ':';
}

/**
 * @brief Read a line from a file stream into a String.
 * @return EOF (-1) if we reached EOF and no data were read. 0 otherwise.
 */
int readline(FILE* fp, String* s) {
  s_clear(s);

  int ch;
  while (true) {
    ch = fgetc(fp);
    if (ch == '\n' || ch == EOF) {
      break;
    }
    s_appendc(s, ch);
  }

  if (s->len == 0 && ch == EOF) return EOF;

  return 0;
}

Parser* parser_create(FILE* f) {
  // TODO: seek to beginning of f.
  Parser* p = malloc(sizeof(Parser));
  p->f = f;
  p->cur_ins = s_new();
  p->next_ins = s_new();
  p->symbol = s_new();
  p->dest = s_new();
  p->comp = s_new();
  p->jump = s_new();
  parser_advance(p);
  return p;
}

bool parser_has_more_lines(Parser* p) { return p->next_ins.len > 0; }

int parser_reset(Parser* p) {
  if (fseek(p->f, 0, SEEK_SET) != 0) {
    perror("error seeking to beginning of file");
    return -1;
  }
  s_clear(&p->cur_ins);
  s_clear(&p->next_ins);
  s_clear(&p->symbol);
  s_clear(&p->dest);
  s_clear(&p->comp);
  s_clear(&p->jump);
  // TODO: having to call parser_advance to ensure parser works correctly is a
  // bit annoying.
  parser_advance(p);
  return 0;
}

/** Read the next instruction from input and make it the current instruction. */
void parser_advance(Parser* p) {
  s_copy(&p->cur_ins, &p->next_ins);

  bool is_empty;
  bool is_comment;
  while (true) {
    int res = readline(p->f, &p->next_ins);
    if (res == EOF) {
      return;
    }
    s_trim(&p->next_ins);
    is_empty = p->next_ins.len == 0;
    is_comment = p->next_ins.len >= 2 && p->next_ins.data[0] == '/' &&
                 p->next_ins.data[1] == '/';
    if (!is_empty && !is_comment) break;
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

const char* parser_symbol(Parser* p) {
  if (parser_instruction_type(p) == A_INSTRUCTION) {
    s_clear(&p->symbol);

    // TODO: properly parse the A instruction (@symbol).
    size_t pos = 1;

    while (pos < p->cur_ins.len && is_symbol_char(p->cur_ins.data[pos])) {
      s_appendc(&p->symbol, p->cur_ins.data[pos]);
      pos++;
    }
    return p->symbol.data;
  }

  if (parser_instruction_type(p) == L_INSTRUCTION) {
    s_clear(&p->symbol);

    // TODO: properly parse the L instruction ((LABEL)).
    size_t pos = 1;
    while (pos < p->cur_ins.len && is_symbol_char(p->cur_ins.data[pos])) {
      s_appendc(&p->symbol, p->cur_ins.data[pos]);
      pos++;
    }
    return p->symbol.data;
  }

  return NULL;
}

const char* parser_dest(Parser* p) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return NULL;
  }

  s_clear(&p->dest);

  size_t pos = 0;
  while (pos < p->cur_ins.len && p->cur_ins.data[pos] != '=') {
    pos++;
  }

  // No equal sign was found.
  if (pos == p->cur_ins.len) {
    return p->dest.data;
  }

  s_substr(&p->cur_ins, 0, pos, &p->dest);
  return p->dest.data;
}

const char* parser_comp(Parser* p) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return NULL;
  }

  s_clear(&p->comp);

  size_t start = 0;
  size_t end = 1;
  while (end < p->cur_ins.len && p->cur_ins.data[end] != ';') {
    if (p->cur_ins.data[end - 1] == '=') {
      start = end;
    }
    end++;
  }

  s_substr(&p->cur_ins, start, end, &p->comp);
  return p->comp.data;
}

const char* parser_jump(Parser* p) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return NULL;
  }

  s_clear(&p->jump);

  size_t start = 0;
  while (start < p->cur_ins.len && p->cur_ins.data[start] != ';') {
    start++;
  }

  s_substr(&p->cur_ins, start + 1, p->cur_ins.len, &p->jump);
  return p->jump.data;
}

void parser_destroy(Parser* p) {
  s_destroy(&p->cur_ins);
  s_destroy(&p->next_ins);
  s_destroy(&p->symbol);
  s_destroy(&p->dest);
  s_destroy(&p->comp);
  s_destroy(&p->jump);
  free(p);
}

typedef struct {
  char* key;
  char* value;
} StrMap;

typedef struct {
  StrMap* dest_mp;
  StrMap* comp_mp;
  StrMap* jump_mp;
} Code;

// TODO: using a map for translation might be too rigid.
StrMap* dest_map_create() {
  StrMap* dest_mp = NULL;
  shput(dest_mp, "", "000");
  shput(dest_mp, "M", "001");
  shput(dest_mp, "D", "010");
  shput(dest_mp, "DM", "011");
  shput(dest_mp, "A", "100");
  shput(dest_mp, "AM", "101");
  shput(dest_mp, "AD", "110");
  shput(dest_mp, "ADM", "111");
  return dest_mp;
}

StrMap* comp_map_create() {
  StrMap* comp_mp = NULL;

  // a == 0
  shput(comp_mp, "0", "0101010");
  shput(comp_mp, "1", "0111111");
  shput(comp_mp, "-1", "0111010");
  shput(comp_mp, "D", "0001100");
  shput(comp_mp, "A", "0110000");
  shput(comp_mp, "!D", "0001101");
  shput(comp_mp, "!A", "0110001");
  shput(comp_mp, "-D", "0001111");
  shput(comp_mp, "-A", "0110011");
  shput(comp_mp, "D+1", "0011111");
  shput(comp_mp, "A+1", "0110111");
  shput(comp_mp, "D-1", "0001110");
  shput(comp_mp, "A-1", "0110010");
  shput(comp_mp, "D+A", "0000010");
  shput(comp_mp, "D-A", "0010011");
  shput(comp_mp, "A-D", "0000111");
  shput(comp_mp, "D&A", "0000000");
  shput(comp_mp, "D|A", "0010101");

  // a == 1: the same c bits, with A replaced by M.
  shput(comp_mp, "M", "1110000");
  shput(comp_mp, "!M", "1110001");
  shput(comp_mp, "-M", "1110011");
  shput(comp_mp, "M+1", "1110111");
  shput(comp_mp, "M-1", "1110010");
  shput(comp_mp, "D+M", "1000010");
  shput(comp_mp, "D-M", "1010011");
  shput(comp_mp, "M-D", "1000111");
  shput(comp_mp, "D&M", "1000000");
  shput(comp_mp, "D|M", "1010101");

  return comp_mp;
}

StrMap* jump_map_create() {
  StrMap* jump_mp = NULL;
  shput(jump_mp, "", "000");
  shput(jump_mp, "JGT", "001");
  shput(jump_mp, "JEQ", "010");
  shput(jump_mp, "JGE", "011");
  shput(jump_mp, "JLT", "100");
  shput(jump_mp, "JNE", "101");
  shput(jump_mp, "JLE", "110");
  shput(jump_mp, "JMP", "111");
  return jump_mp;
}

Code code_create() {
  Code c = {
      .dest_mp = dest_map_create(),
      .comp_mp = comp_map_create(),
      .jump_mp = jump_map_create(),
  };
  return c;
}

char* code_dest(Code c, const char* mnemonic) {
  return shget(c.dest_mp, mnemonic);
}

char* code_comp(Code c, const char* mnemonic) {
  return shget(c.comp_mp, mnemonic);
}

char* code_jump(Code c, const char* mnemonic) {
  return shget(c.jump_mp, mnemonic);
}

void code_destroy(Code* c) {
  shfree(c->dest_mp);
  shfree(c->comp_mp);
  shfree(c->jump_mp);
}

typedef struct {
  char* key;
  int32_t value;
} Entry;

typedef struct {
  Entry* mp;
} SymbolTable;

SymbolTable st_create() {
  Entry* mp = NULL;
  sh_new_strdup(mp);
  shput(mp, "R0", 0);
  shput(mp, "R1", 1);
  shput(mp, "R2", 2);
  shput(mp, "R3", 3);
  shput(mp, "R4", 4);
  shput(mp, "R5", 5);
  shput(mp, "R6", 6);
  shput(mp, "R7", 7);
  shput(mp, "R8", 8);
  shput(mp, "R9", 9);
  shput(mp, "R10", 10);
  shput(mp, "R11", 11);
  shput(mp, "R12", 12);
  shput(mp, "R13", 13);
  shput(mp, "R14", 14);
  shput(mp, "R15", 15);
  shput(mp, "SP", 0);
  shput(mp, "LCL", 1);
  shput(mp, "ARG", 2);
  shput(mp, "THIS", 3);
  shput(mp, "THAT", 4);
  shput(mp, "SCREEN", 16384);
  shput(mp, "KBD", 24576);

  return (SymbolTable){.mp = mp};
}

void st_add_entry(SymbolTable* st, const char* symbol, int32_t address) {
  shput(st->mp, symbol, address);
}

bool st_contains(SymbolTable* st, const char* symbol) {
  int index = shgeti(st->mp, symbol);
  if (index == -1) {
    return false;
  }
  return true;
}

int32_t st_get_address(SymbolTable* st, const char* symbol) {
  if (!st_contains(st, symbol)) {
    return -1;
  }
  return shget(st->mp, symbol);
}

void st_destroy(SymbolTable* st) { shfree(st->mp); }

typedef struct {
  SymbolTable st;
  size_t next_var_addr;
} Assembler;

Assembler assembler_create() {
  SymbolTable st = st_create();
  return (Assembler){.st = st, .next_var_addr = 16};
}

static int32_t assembler_resolve_symbol(Assembler* as, const char* symbol) {
  if (is_digit_string(symbol, strlen(symbol))) {
    // TODO: int automatically casted as int32_t
    return atoi(symbol);
  }
  if (!st_contains(&as->st, symbol)) {
    st_add_entry(&as->st, symbol, as->next_var_addr);
    as->next_var_addr++;
  }
  return st_get_address(&as->st, symbol);
}

int assembler_run(Assembler* as, FILE* out_f, FILE* in_f) {
  int retval = -1;

  Parser* parser = parser_create(in_f);
  size_t line_num = 0;
  InstructionType itype;
  while (parser_has_more_lines(parser)) {
    parser_advance(parser);
    itype = parser_instruction_type(parser);
    if (itype != L_INSTRUCTION) {
      line_num++;
      continue;
    }

    const char* symbol = parser_symbol(parser);
    st_add_entry(&as->st, symbol, line_num);
  }

  if (parser_reset(parser) != 0) {
    perror("error resetting parser");
    goto cleanup;
  }

  Code encoder = code_create();
  char instruction[17];
  while (parser_has_more_lines(parser)) {
    parser_advance(parser);
    itype = parser_instruction_type(parser);

    if (itype == L_INSTRUCTION) {
      continue;
    }

    if (itype == A_INSTRUCTION) {
      const char* symbol = parser_symbol(parser);
      int32_t memory_addr = assembler_resolve_symbol(as, symbol);
      if (memory_addr == -1) {
        goto cleanup;
      }

      for (size_t i = 15; i > 0; i--) {
        instruction[i] = (memory_addr % 2) + '0';
        memory_addr = memory_addr >> 1;
      }
      instruction[0] = '0';
      instruction[16] = '\0';
    } else if (itype == C_INSTRUCTION) {
      snprintf(instruction, sizeof instruction, "111%s%s%s",
               code_comp(encoder, parser_comp(parser)),
               code_dest(encoder, parser_dest(parser)),
               code_jump(encoder, parser_jump(parser)));
    }

    fputs(instruction, out_f);
    if (parser_has_more_lines(parser)) {
      fputc('\n', out_f);
    }
  }

  retval = 0;
cleanup:
  code_destroy(&encoder);
  parser_destroy(parser);
  return retval;
}

void assembler_destroy(Assembler* as) { st_destroy(&as->st); }

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "expect 1 argument, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  char* asm_file_path = argv[1];
  String hack_file_path = s_init(argv[1], strlen(argv[1]));
  s_trim_suffix(&hack_file_path, ".asm");
  s_append(&hack_file_path, ".hack");

  int retval = EXIT_FAILURE;
  FILE* f = fopen(asm_file_path, "r");
  FILE* out_f = fopen(hack_file_path.data, "w");

  Assembler as = assembler_create();
  int result = assembler_run(&as, out_f, f);
  if (result == -1) {
    goto cleanup;
  }

  retval = EXIT_SUCCESS;
cleanup:
  assembler_destroy(&as);
  fclose(out_f);
  fclose(f);
  s_destroy(&hack_file_path);
  return retval;
}
