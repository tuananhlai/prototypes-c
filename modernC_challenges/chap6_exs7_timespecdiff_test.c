#include <stddef.h>
#include <time.h>
#include <unistd.h>
#define UNIT_TEST
#include "../acutest.h"
#include "chap6_exs7_timespecdiff.c"

static bool is_timespec_equal(struct timespec ts1, struct timespec ts2) {
  return ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec == ts2.tv_nsec;
}

void test_timespecdiff() {
  struct {
    struct timespec arg_ts1;
    struct timespec arg_ts2;
    struct timespec expected;
  } test_cases[] = {
      {{3, 0}, {1, 0}, {2, 0}},       {{3, 0}, {3, 0}, {0, 0}},
      {{2, 0}, {3, 0}, {-1, 0}},      {{3, 5000}, {2, 2000}, {1, 3000}},
      {{3, 5000}, {2, 5000}, {1, 0}}, {{3, 2000}, {2, 5000}, {0, 999'997'000}},
  };
  size_t test_case_len = sizeof(test_cases) / sizeof(test_cases[0]);

  for (size_t i = 0; i < test_case_len; i++) {
    struct timespec actual =
        timespec_diff(test_cases[i].arg_ts1, test_cases[i].arg_ts2);
    TEST_CHECK_(is_timespec_equal(actual, test_cases[i].expected),
                "[%zu] result not match\n", i);
  }
}

TEST_LIST = {
    {"timespec_diff", test_timespecdiff},
    {NULL, NULL},
};