#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../acutest.h"

#define UNIT_TEST
#include "vm_translator.c"

void test_translate() {
  struct {
    const char *name;
    const char *vm_code;
    const char *expected_hack;
  } test_cases[] = {
      {
          .name = "should translate 'push constant'",
          .vm_code = "push constant 7\n",
          .expected_hack = "@7\n"
                           "D=A\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push local'",
          .vm_code = "push local 2\n",
          .expected_hack = "@LCL\n"
                           "D=M\n"
                           "@2\n"
                           "A=D+A\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop local'",
          .vm_code = "pop local 2\n",
          .expected_hack = "@2\n"
                           "D=A\n"
                           "@LCL\n"
                           "D=D+M\n"
                           "@R13\n"
                           "M=D\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@R13\n"
                           "A=M\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push argument'",
          .vm_code = "push argument 1\n",
          .expected_hack = "@ARG\n"
                           "D=M\n"
                           "@1\n"
                           "A=D+A\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop argument'",
          .vm_code = "pop argument 1\n",
          .expected_hack = "@1\n"
                           "D=A\n"
                           "@ARG\n"
                           "D=D+M\n"
                           "@R13\n"
                           "M=D\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@R13\n"
                           "A=M\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push this'",
          .vm_code = "push this 3\n",
          .expected_hack = "@THIS\n"
                           "D=M\n"
                           "@3\n"
                           "A=D+A\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop this'",
          .vm_code = "pop this 3\n",
          .expected_hack = "@3\n"
                           "D=A\n"
                           "@THIS\n"
                           "D=D+M\n"
                           "@R13\n"
                           "M=D\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@R13\n"
                           "A=M\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push that'",
          .vm_code = "push that 4\n",
          .expected_hack = "@THAT\n"
                           "D=M\n"
                           "@4\n"
                           "A=D+A\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop that'",
          .vm_code = "pop that 4\n",
          .expected_hack = "@4\n"
                           "D=A\n"
                           "@THAT\n"
                           "D=D+M\n"
                           "@R13\n"
                           "M=D\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@R13\n"
                           "A=M\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push temp'",
          .vm_code = "push temp 6\n",
          .expected_hack = "@11\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop temp'",
          .vm_code = "pop temp 6\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@11\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push pointer 0'",
          .vm_code = "push pointer 0\n",
          .expected_hack = "@THIS\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push pointer 1'",
          .vm_code = "push pointer 1\n",
          .expected_hack = "@THAT\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop pointer 0'",
          .vm_code = "pop pointer 0\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@THIS\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop pointer 1'",
          .vm_code = "pop pointer 1\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@THAT\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'push static'",
          .vm_code = "push static 5\n",
          .expected_hack = "@Test.5\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'pop static'",
          .vm_code = "pop static 5\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@Test.5\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'add'",
          .vm_code = "add\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=D+M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'eq'",
          .vm_code = "eq\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M-D\n"
                           "@L0\n"
                           "D;JEQ\n"
                           "@L1\n"
                           "D=0;JMP\n"
                           "(L0)\n"
                           "D=-1\n"
                           "(L1)\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'gt'",
          .vm_code = "gt\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M-D\n"
                           "@L0\n"
                           "D;JGT\n"
                           "@L1\n"
                           "D=0;JMP\n"
                           "(L0)\n"
                           "D=-1\n"
                           "(L1)\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'lt'",
          .vm_code = "lt\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M-D\n"
                           "@L0\n"
                           "D;JLT\n"
                           "@L1\n"
                           "D=0;JMP\n"
                           "(L0)\n"
                           "D=-1\n"
                           "(L1)\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'not'",
          .vm_code = "not\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=!M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'neg'",
          .vm_code = "neg\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=-M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'sub'",
          .vm_code = "sub\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=M-D\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'and'",
          .vm_code = "and\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=D&M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'or'",
          .vm_code = "or\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@SP\n"
                           "AM=M-1\n"
                           "D=D|M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name = "should translate 'label'",
          .vm_code = "label TEST\n",
          .expected_hack = "(TEST)\n",
      },
      {
          .name = "should translate 'goto'",
          .vm_code = "goto TEST\n",
          .expected_hack = "@TEST\n"
                           "0;JMP\n",
      },
      {
          // regression test for incorrect format specifier (%*s vs %.*s)
          .name = "should not include comment in Hack ASM output",
          .vm_code = "label TEST // comment\n",
          .expected_hack = "(TEST)\n",
      },
      {
          .name = "should translate 'if-goto'",
          .vm_code = "if-goto TEST\n",
          .expected_hack = "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@TEST\n"
                           "D;JNE\n",
      },
      {
          .name = "should translate 'call'",
          .vm_code = "call Sys.main 2",
          .expected_hack = "@Sys.main$ret.0\n" // push return address
                           "D=A\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n"
                           "@LCL\n" // push LCL base address
                           "D=M\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n"
                           "@ARG\n" // push ARG base address
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
                           "@7\n"
                           "D=A\n"
                           "@ARG\n" // set ARG and LCL for callee
                           "M=A-D\n"
                           "@SP\n"
                           "D=M\n"
                           "@LCL\n"
                           "M=D\n"
                           "@Sys.main\n" // jump to the target function
                           "0;JMP\n"
                           "(Sys.main$ret.0)",
      },
      {
          .name = "should translate function declaration",
          .vm_code = "function Sys.main 2\nreturn",
          .expected_hack = "(Sys.main)\n"
                           "@SP\n" // push 2 local variable into the stack
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=0\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=0\n"
                           "@2\n"    // number of local variables
                           "D=A+1\n" // pop all local variables + the last entry
                                     // of the latest function frame
                           "@SP\n"
                           "AM=M-D\n"
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
                           "A=M\n"
                           "0;JMP\n",
      },
      {
          .name = "should output nothing for empty input",
          .vm_code = "",
          .expected_hack = "",
      },
      {
          .name = "should ignore comment line",
          .vm_code = "// comment",
          .expected_hack = "",
      },
      {
          .name = "should ignore extra whitespaces",
          .vm_code = "   push     constant 999 //comment    \n",
          .expected_hack = "@999\n"
                           "D=A\n"
                           "@SP\n"
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n",
      },
      {
          .name =
              "should output nothing for line with only whitespace characters",
          .vm_code = "             ",
          .expected_hack = "",
      }};
  size_t test_case_len = sizeof(test_cases) / sizeof(test_cases[0]);

  for (size_t i = 0; i < test_case_len; i++) {
    FILE *in = fmemopen((void *)test_cases[i].vm_code,
                        strlen(test_cases[i].vm_code), "r");
    char *out_buf = NULL;
    size_t out_len = 0;
    FILE *out = open_memstream(&out_buf, &out_len);

    int res = translate("Test", in, out);
    fclose(in);
    fclose(out);

    if (!TEST_CHECK_(res == 0, "[case %zu] translate() succeeded", i)) {
      TEST_MSG("error: %s", error_get());
      free(out_buf);
      continue;
    }

    if (!TEST_CHECK_(strcmp(out_buf, test_cases[i].expected_hack) == 0, "%s",
                     test_cases[i].name)) {
      TEST_MSG("expected:\n%s", test_cases[i].expected_hack);
      TEST_MSG("actual:\n%s", out_buf);
    }

    free(out_buf);
  }
}

TEST_LIST = {
    {"test translate", test_translate},
    {NULL, NULL},
};
