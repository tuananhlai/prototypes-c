#include "../acutest.h"
#include <stddef.h>
#include <string.h>
#include <time.h>

#define UNIT_TEST
#include "chap8_exs1_my-strtod.c"

void test_mystrtod() {
  struct {
    char *arg_nptr;
    double expected;
    char *expected_endptr;
  } test_cases[] = {
      // whole subject sequence consumed: endptr lands on the '\0'
      {"0", 0, ""},
      {"-1", -1, ""},
      {"4.5", 4.5, ""},
      {"+1.333", 1.333, ""},
      {"-29.11", -29.11, ""},
      {"  -29.11", -29.11, ""},
      // conversion stops at the first character that can't extend the number
      {"-29.1a1", -29.1, "a1"},
      {"+12.x", 12, ".x"},
      {"5p2.1", 5, "p2.1"},
      {"u", 0, "u"},
      {"-u1", 0, "-u1"},
  };
  size_t test_case_len = sizeof(test_cases) / sizeof(test_cases[0]);

  for (size_t i = 0; i < test_case_len; i++) {
    const char *got_endptr;
    double got = my_strtod(test_cases[i].arg_nptr, &got_endptr);
    TEST_CHECK_(test_cases[i].expected == got,
                "[%zu] retval: expected %.17f, got %.17f", i,
                test_cases[i].expected, got);
    TEST_CHECK_(strcmp(got_endptr, test_cases[i].expected_endptr) == 0,
                "[%zu] endptr: expected %s, got %s", i,
                test_cases[i].expected_endptr, got_endptr);
  }
}

TEST_LIST = {
    {"test strtod", test_mystrtod},
    {NULL, NULL},
};
