#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../acutest.h"

#define UNIT_TEST
#include "vm_translator.c"

void test_translate() {
  struct {
    const char *vm_code;
    const char *expected_hack;
  } test_cases[] = {{
                        .vm_code = "push constant 7\n",
                        .expected_hack = "@7\n"
                                         "D=A\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "push local 2\n",
                        .expected_hack = "@LCL\n"
                                         "D=M\n"
                                         "@2\n"
                                         "A=D+A\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
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
                        .vm_code = "push argument 1\n",
                        .expected_hack = "@ARG\n"
                                         "D=M\n"
                                         "@1\n"
                                         "A=D+A\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
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
                        .vm_code = "push this 3\n",
                        .expected_hack = "@THIS\n"
                                         "D=M\n"
                                         "@3\n"
                                         "A=D+A\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
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
                        .vm_code = "push that 4\n",
                        .expected_hack = "@THAT\n"
                                         "D=M\n"
                                         "@4\n"
                                         "A=D+A\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
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
                        .vm_code = "push temp 6\n",
                        .expected_hack = "@11\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "pop temp 6\n",
                        .expected_hack = "@SP\n"
                                         "AM=M-1\n"
                                         "D=M\n"
                                         "@11\n"
                                         "M=D\n",
                    },
                    {
                        .vm_code = "push pointer 0\n",
                        .expected_hack = "@THIS\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "push pointer 1\n",
                        .expected_hack = "@THAT\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "pop pointer 0\n",
                        .expected_hack = "@SP\n"
                                         "AM=M-1\n"
                                         "D=M\n"
                                         "@THIS\n"
                                         "M=D\n",
                    },
                    {
                        .vm_code = "pop pointer 1\n",
                        .expected_hack = "@SP\n"
                                         "AM=M-1\n"
                                         "D=M\n"
                                         "@THAT\n"
                                         "M=D\n",
                    },
                    {
                        .vm_code = "push static 5\n",
                        .expected_hack = "@Test.5\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "pop static 5\n",
                        .expected_hack = "@SP\n"
                                         "AM=M-1\n"
                                         "D=M\n"
                                         "@Test.5\n"
                                         "M=D\n",
                    },
                    {
                        .vm_code = "add\n",
                        .expected_hack = "@SP\n"
                                         "A=M\n"
                                         "D=M\n"
                                         "@SP\n"
                                         "M=M-1\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "D=D+M\n"
                                         "@SP\n"
                                         "M=M-1\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
                    },
                    {
                        .vm_code = "",
                        .expected_hack = "",
                    },
                    {
                        .vm_code = "// comment",
                        .expected_hack = "",
                    },
                    {
                        .vm_code = "   push     constant 999 //comment    \n",
                        .expected_hack = "@999\n"
                                         "D=A\n"
                                         "@SP\n"
                                         "A=M\n"
                                         "M=D\n"
                                         "@SP\n"
                                         "M=M+1\n",
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

    if (!TEST_CHECK_(strcmp(out_buf, test_cases[i].expected_hack) == 0,
                     "[case %zu] output matches expected", i)) {
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
