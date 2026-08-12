#include <stdio.h>
#include <stdlib.h>

int* dup(int arr[], size_t n);

int main() {
  // Statically allocated array
  int arr[3] = {1, 2, 3};
  size_t arr_len = sizeof arr / sizeof(int);
  int* new_arr = dup(arr, arr_len);
  puts("new_arr:");
  for (size_t i = 0; i < arr_len; i++) {
    printf("%d\n", new_arr[i]);
  }

  // Dynamically allocated array
  size_t arr_1_len = 5;
  int* arr_1 = malloc(arr_1_len * sizeof(int));
  for (size_t i = 0; i < arr_1_len; i++) {
    arr_1[i] = arr_1_len - i;
  }

  int* new_arr_1 = dup(arr_1, arr_1_len);
  puts("new_arr_1:");
  for (size_t i = 0; i < arr_1_len; i++) {
    printf("%d\n", new_arr_1[i]);
  }

  free(new_arr_1);
  free(arr_1);
  free(new_arr);
  return 0;
}

int* dup(int arr[], size_t n) {
  int* new_arr = malloc(n * sizeof(int));
  for (size_t i = 0; i < n; i++) {
    new_arr[i] = arr[i];
  }
  return new_arr;
}
