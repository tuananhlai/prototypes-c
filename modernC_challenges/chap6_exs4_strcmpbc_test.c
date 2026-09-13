
#include "../acutest.h"

#define UNIT_TEST
#include "chap6_exs4_strcmpbc.c"

typedef enum {
  NEG,
  ZERO,
  POS,
} Result;

static const char *result_names[] = {"NEG", "ZERO", "POS"};

static Result to_result(int cmp) {
  if (cmp < 0)
    return NEG;
  if (cmp > 0)
    return POS;
  return ZERO;
}

void test_strcmpbc() {
  struct {
    char *s1;
    char *s2;
    size_t n;
    Result expected;
  } test_cases[] = {
      {"abc", "abc", 4, ZERO},
      {"abc", "abc", 5, ZERO},
      {"abc\0a", "abc\0b", 10, ZERO},
      {"abc", "abd", 5, NEG},
      {"abd", "abc", 5, POS},
      {"abc", "abd", 2, ZERO},
      {"", "a", 10, NEG},
      {(char[]){'a', 'b'}, (char[]){'a', 'c'}, 2, NEG},
      {(char[]){'a', 'b'}, (char[]){'a', 'b'}, 2, ZERO},
      {(char[]){'a', 'b', 'c'}, "a", 3, POS},
  };
  size_t test_case_len = sizeof(test_cases) / sizeof(test_cases[0]);

  for (size_t i = 0; i < test_case_len; i++) {
    Result actual = to_result(
        strcmpbc(test_cases[i].s1, test_cases[i].s2, test_cases[i].n));
    TEST_CHECK_(actual == test_cases[i].expected, "[%zu] expected %s, got %s",
                i, result_names[test_cases[i].expected], result_names[actual]);
  }
}

TEST_LIST = {
    {"bound-checking strcmp", test_strcmpbc},
    {NULL, NULL},
};
