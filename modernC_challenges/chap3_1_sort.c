#include <stdio.h>
#include <stdlib.h>

void selection_sort(int arr[], size_t n);
void insertion_sort(int arr[], size_t n);
void merge(int arr[], size_t lo1, size_t hi1, size_t hi2);
void merge_sort(int arr[], size_t n);
void merge_sort_recursive(int arr[], size_t lo, size_t hi);
void quick_sort(int arr[], size_t n);
void quick_sort_recursive(int arr[], size_t lo, size_t hi);
void swap(int arr[], size_t i, size_t j);

int main(int argc, char const* argv[]) {
  // int arr[] = {10, 5, 2, 6, 3, 1, 2, 8};
  // size_t arr_len = sizeof(arr) / sizeof(int);
  // selection_sort(arr, arr_len);

  // for (int i = 0; i < arr_len; i++) {
  //   printf("%d\n", arr[i]);
  // }

  int arr2[] = {10, 5, 2, 6, 3, 1, 2, 8};
  quick_sort(arr2, 8);
  for (int i = 0; i < 8; i++) {
    printf("%d\n", arr2[i]);
  }

  return EXIT_SUCCESS;
}

void insertion_sort(int arr[], size_t n) {
  int j, tmp;
  for (int i = 1; i < n; i++) {
    j = i;

    while (j > 0 && arr[j] < arr[j - 1]) {
      swap(arr, j, j - 1);
      j--;
    }
  }
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
    swap(arr, i, min_idx);
  }
}

void merge_sort(int arr[], size_t n) { merge_sort_recursive(arr, 0, n); }

void merge_sort_recursive(int arr[], size_t lo, size_t hi) {
  if (hi <= lo + 1) return;
  size_t mid = (lo + hi) / 2;
  merge_sort_recursive(arr, lo, mid);
  merge_sort_recursive(arr, mid, hi);
  size_t arr1_len = mid - lo;
  size_t arr2_len = hi - mid;
  merge(arr, lo, mid, hi);
}

void merge(int arr[], size_t lo1, size_t hi1, size_t hi2) {
  int sorted_arr_len = hi2 - lo1;
  int sorted_arr[sorted_arr_len];

  int i = lo1, j = hi1, k = 0;
  while (i < hi1 || j < hi2) {
    if (i >= hi1) {
      sorted_arr[k] = arr[j];
      j++;
    } else if (j >= hi2) {
      sorted_arr[k] = arr[i];
      i++;
    } else if (arr[j] < arr[i]) {
      sorted_arr[k] = arr[j];
      j++;
    } else {
      sorted_arr[k] = arr[i];
      i++;
    }

    k++;
  }

  for (int i = lo1; i < hi2; i++) {
    arr[i] = sorted_arr[i - lo1];
  }
}

void quick_sort(int arr[], size_t n) { quick_sort_recursive(arr, 0, n); }

void quick_sort_recursive(int arr[], size_t lo, size_t hi) {
  if (hi <= lo + 1) return;

  int anchor = arr[lo];
  int i = lo + 1;
  for (int j = i; j < hi; j++) {
    if (arr[j] >= anchor) {
      continue;
    }

    swap(arr, i, j);
    i++;
  }

  swap(arr, lo, i - 1);
  quick_sort_recursive(arr, lo, i);
  quick_sort_recursive(arr, i, hi);
}

void swap(int arr[], size_t i, size_t j) {
  int tmp = arr[i];
  arr[i] = arr[j];
  arr[j] = tmp;
}