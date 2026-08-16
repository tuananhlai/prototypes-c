#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  int32_t minus_one = 0xffff'ffff;
  printf("0xFFFFFFFF as signed int32_t: %d\n", minus_one);

  int32_t minus_two_bil = 0x8000'0000;
  printf("0x80000000 (INT32_MIN) as signed int32_t: %d\n", minus_two_bil);

  uint32_t bits = 0b0'0111'1111'1100'0000'0000'0000'0000'000;
  float f;
  memcpy(&f, &bits, sizeof(uint32_t));
  printf("IEEE 754 bit pattern (0x%08X) as float: %f\n", bits, f);

  f = -0.3;
  memcpy(&bits, &f, sizeof(uint32_t));
  printf("Float %f as IEEE 754 binary [Sign Exp Mantissa]: %b %08b %023b\n",
         f, bits >> 31, (bits >> 23) & 0xFF, bits & 0x7FFFFF);

  int32_t overflowed = INT32_MAX + 1;
  printf("INT32_MAX + 1 (signed overflow wrap): %d\n", overflowed);

  int32_t positive_val = 2911;
  int32_t negative_val = ~positive_val + 1;
  printf("Two's complement negation (~%d + 1): %d\n", positive_val,
         negative_val);

  printf("sizeof( 2,147,483,647) [fits in int32]: %zu bytes\n",
         sizeof(2'147'483'647));
  printf("sizeof( 2,147,483,648) [promoted type]: %zu bytes\n",
         sizeof(2'147'483'648));
  printf("sizeof(-2,147,483,648) [unary - on promoted type]: %zu bytes\n",
         sizeof(-2'147'483'648));

  return 0;
}
