#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynamic_string.h"

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

int main(void) {
  const char* text = "   \n @10  \nD=0;JMP\n@2023 \n M=A \n 0;JMP";
  FILE* f = fmemopen((void*)text, strlen(text), "r");

  Parser* p = parser_open(f);

  String symbol = s_new();
  String dest = s_new();
  String comp = s_new();
  String jump = s_new();
  while (parser_has_more_lines(p)) {
    parser_advance(p);
    if (parser_instruction_type(p) == A_INSTRUCTION) {
      parser_symbol(p, &symbol);
      puts(symbol.data);
      continue;
    }

    if (parser_instruction_type(p) == C_INSTRUCTION) {
      parser_dest(p, &dest);
      puts(dest.data);
      parser_comp(p, &comp);
      puts(comp.data);
      parser_jump(p, &jump);
      puts(jump.data);
      continue;
    }
  }

  // Clean up.
  s_destroy(&jump);
  s_destroy(&comp);
  s_destroy(&dest);
  s_destroy(&symbol);
  parser_destroy(p);
  fclose(f);
  return EXIT_SUCCESS;
}
