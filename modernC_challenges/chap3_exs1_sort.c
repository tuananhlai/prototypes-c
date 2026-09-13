#include <stdio.h>
#include <stdlib.h>

void selection_sort(int arr[], size_t n);
void insertion_sort(int arr[], size_t n);
void insertion_sort_range(int arr[], size_t lo, size_t hi);
void merge(int arr[], size_t lo1, size_t hi1, size_t hi2);
void merge_sort(int arr[], size_t n);
void merge_sort_recursive(int arr[], size_t lo, size_t hi);
void quick_sort(int arr[], size_t n);
void quick_sort_recursive(int arr[], size_t lo, size_t hi);
void swap(int arr[], size_t i, size_t j);
void print_array(int arr[]);

const int CUTOFF = 5;

int main() {
  int arr[] = {10, 5, 2, 6, 3, 1, 2, 8};
  merge_sort(arr, 8);
  print_array(arr);

  return EXIT_SUCCESS;
}

void insertion_sort(int arr[], size_t n) { insertion_sort_range(arr, 0, n); }

void insertion_sort_range(int arr[], size_t lo, size_t hi) {
  size_t j;
  for (size_t i = lo + 1; i < hi; i++) {
    j = i;

    while (j > lo && arr[j] < arr[j - 1]) {
      swap(arr, j, j - 1);
      j--;
    }
  }
}

void selection_sort(int arr[], size_t n) {
  size_t min_idx;
  for (size_t i = 0; i < n; i++) {
    min_idx = i;
    for (size_t j = i + 1; j < n; j++) {
      if (arr[j] < arr[min_idx]) {
        min_idx = j;
      }
    }
    swap(arr, i, min_idx);
  }
}

void merge_sort(int arr[], size_t n) { merge_sort_recursive(arr, 0, n); }

void merge_sort_recursive(int arr[], size_t lo, size_t hi) {
  if (hi <= lo + CUTOFF) {
    insertion_sort_range(arr, lo, hi);
    return;
  }

  size_t mid = (lo + hi) / 2;
  merge_sort_recursive(arr, lo, mid);
  merge_sort_recursive(arr, mid, hi);

  merge(arr, lo, mid, hi);
}

void merge(int arr[], size_t lo1, size_t hi1, size_t hi2) {
  size_t sorted_arr_len = hi2 - lo1;
  int sorted_arr[sorted_arr_len];

  size_t i = lo1, j = hi1, k = 0;
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

  for (size_t i = lo1; i < hi2; i++) {
    arr[i] = sorted_arr[i - lo1];
  }
}

void quick_sort(int arr[], size_t n) { quick_sort_recursive(arr, 0, n); }

void quick_sort_recursive(int arr[], size_t lo, size_t hi) {
  if (hi <= lo + CUTOFF) {
    insertion_sort_range(arr, lo, hi);
    return;
  }

  int anchor = arr[lo];
  size_t i = lo + 1;
  for (size_t j = i; j < hi; j++) {
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

void print_array(int arr[]) {
  for (int i = 0; i < 8; i++) {
    printf("%d\n", arr[i]);
  }
}
