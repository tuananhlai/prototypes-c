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
  if (parser_instruction_type(p) != A_INSTRUCTION) {
    return NULL;
  }

  s_clear(&p->symbol);

  size_t pos = 0;
  if (p->cur_ins.data[pos] != '@') {
    return NULL;
  }
  pos++;

  while (pos < p->cur_ins.len && p->cur_ins.data[pos] >= '0' &&
         p->cur_ins.data[pos] <= '9') {
    s_appendc(&p->symbol, p->cur_ins.data[pos]);
    pos++;
  }

  return p->symbol.data;
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

int main(void) {
  FILE* f = fopen("tmp/in.hack", "r");
  FILE* out_f = fopen("tmp/out.asm", "w");

  Parser* p = parser_create(f);
  Code c = code_create();

  char instruction[17];
  while (parser_has_more_lines(p)) {
    parser_advance(p);
    if (parser_instruction_type(p) == A_INSTRUCTION) {
      int memory_addr = atoi(parser_symbol(p));
      for (int i = 15; i > 0; i--) {
        instruction[i] = (memory_addr % 2) + '0';
        memory_addr = memory_addr >> 1;
      }
      instruction[0] = '0';
      instruction[16] = '\0';
    } else if (parser_instruction_type(p) == C_INSTRUCTION) {
      snprintf(instruction, sizeof instruction, "111%s%s%s",
               code_comp(c, parser_comp(p)), code_dest(c, parser_dest(p)),
               code_jump(c, parser_jump(p)));
    }

    fputs(instruction, out_f);
    if (parser_has_more_lines(p)) {
      fputc('\n', out_f);
    }
  }

  // Clean up.
  code_destroy(&c);
  parser_destroy(p);
  fclose(out_f);
  fclose(f);
  return EXIT_SUCCESS;
}
