#define _GNU_SOURCE
#include <stddef.h>
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
} Parser;

int readline(FILE* fp, String* s);

Parser* parser_open(FILE* f);
bool parser_has_more_lines(Parser* p);
void parser_advance(Parser* p);
InstructionType parser_instruction_type(Parser* p);
int parser_symbol(Parser* p, String* s);
int parser_dest(Parser* p, String* out);
int parser_comp(Parser* p, String* out);
int parser_jump(Parser* p, String* out);
void parser_destroy(Parser* p);

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

Parser* parser_open(FILE* f) {
  Parser* p = malloc(sizeof(Parser));
  p->f = f;
  p->cur_ins = s_new();
  p->next_ins = s_new();
  parser_advance(p);
  return p;
}

bool parser_has_more_lines(Parser* p) { return p->next_ins.len > 0; }

void parser_advance(Parser* p) {
  s_copy(&p->cur_ins, &p->next_ins);

  while (true) {
    int res = readline(p->f, &p->next_ins);
    if (res == EOF) {
      return;
    }
    s_trim(&p->next_ins);
    if (p->next_ins.len != 0) {
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

  s_clear(s);

  size_t pos = 0;
  if (p->cur_ins.data[pos] != '@') {
    return -1;
  }
  pos++;

  while (pos < p->cur_ins.len && p->cur_ins.data[pos] >= '0' &&
         p->cur_ins.data[pos] <= '9') {
    s_appendc(s, p->cur_ins.data[pos]);
    pos++;
  }

  return 0;
}

int parser_dest(Parser* p, String* out) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return -1;
  }

  s_clear(out);

  size_t pos = 0;
  while (pos < p->cur_ins.len && p->cur_ins.data[pos] != '=') {
    pos++;
  }

  // No equal sign was found.
  if (pos == p->cur_ins.len) {
    return 0;
  }

  s_substr(&p->cur_ins, 0, pos, out);
  return 0;
}

int parser_comp(Parser* p, String* out) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return -1;
  }

  s_clear(out);

  size_t start = 0;
  size_t end = 1;
  while (end < p->cur_ins.len && p->cur_ins.data[end] != ';') {
    if (p->cur_ins.data[end - 1] == '=') {
      start = end;
    }
    end++;
  }

  s_substr(&p->cur_ins, start, end, out);
  return 0;
}

int parser_jump(Parser* p, String* out) {
  if (parser_instruction_type(p) != C_INSTRUCTION) {
    return -1;
  }

  s_clear(out);

  size_t start = 0;
  while (start < p->cur_ins.len && p->cur_ins.data[start] != ';') {
    start++;
  }

  s_substr(&p->cur_ins, start + 1, p->cur_ins.len, out);
  return 0;
}

void parser_destroy(Parser* p) {
  s_destroy(&p->cur_ins);
  s_destroy(&p->next_ins);
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

char* code_dest(Code c, char* mnemonic) { return shget(c.dest_mp, mnemonic); }

char* code_comp(Code c, char* mnemonic) { return shget(c.comp_mp, mnemonic); }

char* code_jump(Code c, char* mnemonic) { return shget(c.jump_mp, mnemonic); }

void code_destroy(Code* c) {
  shfree(c->dest_mp);
  shfree(c->comp_mp);
  shfree(c->jump_mp);
}

/** Write a 16-bit binary A instruction from the given symbol (i. "1234") to `out`. */
void a_instruction(char* symbol, char out[17]) {
  int memory_addr = atoi(symbol);
  for (int i = 15; i >= 0; i--) {
    out[i] = (memory_addr % 2) + '0';
    memory_addr = memory_addr >> 1;
  }
  out[16] = '\0';
}

/** Write a 16-bit binary C instruction from the binary representation of comp, dest and jump to `out`. */
void c_instruction(char* comp, char* dest, char* jump, char out[17]) {
  snprintf(out, 17, "111%s%s%s", comp, dest, jump);
}

int main(void) {
  FILE* f = fopen("tmp/in.hack", "r");
  FILE* out_f = fopen("tmp/out.asm", "w");

  Parser* p = parser_open(f);
  Code c = code_create();

  String symbol = s_new();
  String dest = s_new();
  String comp = s_new();
  String jump = s_new();
  char instruction[17];
  while (parser_has_more_lines(p)) {
    parser_advance(p);
    if (parser_instruction_type(p) == A_INSTRUCTION) {
      parser_symbol(p, &symbol);
      a_instruction(symbol.data, instruction);
    } else if (parser_instruction_type(p) == C_INSTRUCTION) {
      parser_dest(p, &dest);
      parser_comp(p, &comp);
      parser_jump(p, &jump);
      c_instruction(code_comp(c, comp.data), code_dest(c, dest.data),
                    code_jump(c, jump.data), instruction);
    }

    fputs(instruction, out_f);
    if (parser_has_more_lines(p)) {
      fputc('\n', out_f);
    }
  }

  // Clean up.
  s_destroy(&jump);
  s_destroy(&comp);
  s_destroy(&dest);
  s_destroy(&symbol);
  code_destroy(&c);
  parser_destroy(p);
  fclose(out_f);
  fclose(f);
  return EXIT_SUCCESS;
}
