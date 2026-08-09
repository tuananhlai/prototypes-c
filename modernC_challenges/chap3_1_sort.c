#include <stdio.h>
#include <stdlib.h>

void selection_sort(int arr[], size_t n);

int main(int argc, char const* argv[]) {
  int arr[] = {10, 5, 2, 6, 3, 1, 2, 8};
  size_t arr_len = sizeof(arr) / sizeof(int);
  selection_sort(arr, arr_len);

  for (int i = 0; i < arr_len; i++) {
    printf("%d\n", arr[i]);
  }
  return EXIT_SUCCESS;
}

void selection_sort(int arr[], size_t n) {
  int min_idx;
  for (int i = 0; i < n; i++) {
    min_idx = i;
    for (int j = i + 1; j < n; j++) {
      if (arr[j] < arr[min_idx]) {
        min_idx = j;
      }
    }
    int tmp = arr[i];
    arr[i] = arr[min_idx];
    arr[min_idx] = tmp;
  }
}
