#include <stdio.h>
#include <stdlib.h>

// Exactly one translation unit must define this before including the header.
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
  int key;
  int value;
} IntMap;

typedef struct {
  char* key;
  int value;
} StrMap;

int main(void) {
  // Dynamic array. A NULL pointer is a valid empty array, so there is no init.
  int* nums = NULL;
  arrput(nums, 10);
  arrput(nums, 20);
  arrput(nums, 30);

  // arrlen() returns ptrdiff_t, hence "%td".
  printf("nums: len=%td cap=%td\n", arrlen(nums), arrcap(nums));
  for (ptrdiff_t i = 0; i < arrlen(nums); i++) {
    printf("  nums[%td] = %d\n", i, nums[i]);
  }

  arrdel(nums, 0);  // Remove the first element, shifting the rest down.
  printf("after arrdel(0): nums[0]=%d len=%td\n", nums[0], arrlen(nums));
  arrfree(nums);

  // Hash map keyed by int. Also starts life as NULL.
  IntMap* squares = NULL;
  for (int i = 1; i <= 3; i++) {
    hmput(squares, i, i * i);
  }

  // hmget() may reassign `squares`, so it must not share an expression with
  // another use of it (clang catches this as -Wunsequenced).
  int two = hmget(squares, 2);
  printf("squares: len=%td 2->%d\n", hmlen(squares), two);

  // hmgeti() returns -1 for a missing key; hmget() would return a zeroed value.
  printf("  key 99 index = %td\n", hmgeti(squares, 99));

  // Iterate: entries are stored in a plain array, in insertion order.
  for (ptrdiff_t i = 0; i < hmlen(squares); i++) {
    printf("  %d -> %d\n", squares[i].key, squares[i].value);
  }
  hmfree(squares);

  // Hash map keyed by a string. Note the sh* prefix instead of hm*.
  StrMap* ages = NULL;
  shput(ages, "ada", 36);
  shput(ages, "alan", 41);

  int ada = shget(ages, "ada");
  printf("ages: len=%td ada->%d\n", shlen(ages), ada);
  shfree(ages);

  return EXIT_SUCCESS;
}
