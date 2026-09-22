#include "../acutest.h"
#include <stddef.h>
#include <time.h>

#define UNIT_TEST
#include "chap8_exs1_my-strtod.c"

void test_mystrtod() {
  struct {
    char *arg_nptr;
    double expected;
  } test_cases[] = {
      {"0", 0},
      {"-1", -1},
      {"4.5", 4.5},
      {"-29.11", -29.11},
  };
  size_t test_case_len = sizeof(test_cases) / sizeof(test_cases[0]);

  for (size_t i = 0; i < test_case_len; i++) {
    double got = my_strtod(test_cases[i].arg_nptr, NULL);
    TEST_CHECK_(test_cases[i].expected == got, "[%zu] expected %.17f, got %.17f", i,
                test_cases[i].expected, got);
  }
}

TEST_LIST = {
    {"test strtod", test_mystrtod},
    {NULL, NULL},
};
