#include <stdio.h>
#include <stdlib.h>

int main() {
  size_t count = 3;
  int *arr = malloc(count * sizeof(int));

  // Doesn't work as expected.
  // printf("%zu\n", sizeof(arr) / sizeof(int));

  for (size_t i = 0; i < count; i++) {
    arr[i] = count - i;
  }

  for (size_t i = 0; i < count; i++) {
    printf("%d\n", arr[i]);
  }

  free(arr);

  // printf("%d\n", arr[1]);
  return 0;
}
