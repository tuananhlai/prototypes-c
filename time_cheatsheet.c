#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  printf("%ld, %ld\n", ts.tv_sec, ts.tv_nsec);
  clock_gettime(CLOCK_REALTIME_COARSE, &ts);
  printf("%ld, %ld\n", ts.tv_sec, ts.tv_nsec);
  return EXIT_SUCCESS;
}