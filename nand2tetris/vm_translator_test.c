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
  } test_cases[] = {
      {
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
      {.vm_code = "pop local 2",
       .expected_hack = "@2\nD=A\n@LCL\nD=D+M\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@"
                        "R13\nA=M\nM=D\n"},
  };
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
