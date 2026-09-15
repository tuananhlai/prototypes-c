#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned char a;
  unsigned i;
  unsigned long long ll;
} Struct1;

typedef struct {
  unsigned char a;
  unsigned long long ll;
  unsigned i;
} Struct2;

typedef struct {
  unsigned i;
  unsigned char a;
  unsigned long long ll;
} Struct3;

typedef struct {
  unsigned i;
  unsigned long long ll;
  unsigned char a;
} Struct4;

typedef struct {
  unsigned long long ll;
  unsigned char a;
  unsigned i;
} Struct5;

typedef struct {
  unsigned long long ll;
  unsigned i;
  unsigned char a;
} Struct6;

/**
Create six different structure types for each possibility to order three fields
inside a structure: one unsigned char, one unsigned and one unsigned long long.
Print the sizes of these six structures, they should be significantly different.
Compute the minimal size as the sum of the sizes of each member. Is there any of
your structure that has this size? Which of your structures comes closest to
that ideal size?
 */
int main(void) {
  printf("char: %zu\n", sizeof(unsigned char));
  printf("int: %zu\n", sizeof(unsigned));
  printf("long: %zu\n", sizeof(unsigned long long));
  printf("minimum size: %zu\n",
         sizeof(unsigned long long) + sizeof(unsigned) + sizeof(unsigned char));
  printf("struct(char, int, long): %zu\n", sizeof(Struct1));
  printf("struct(char, long, int): %zu\n", sizeof(Struct2));
  printf("struct(int, char, long): %zu\n", sizeof(Struct3));
  printf("struct(int, long, char): %zu\n", sizeof(Struct4));
  printf("struct(long, char, int): %zu\n", sizeof(Struct5));
  printf("struct(long, int, char): %zu\n", sizeof(Struct6));
  return EXIT_SUCCESS;
}