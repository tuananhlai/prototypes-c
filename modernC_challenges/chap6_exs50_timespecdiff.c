#include <time.h>

struct timespec timespec_diff(struct timespec ts1, struct timespec ts2) {
  if (ts1.tv_nsec < ts2.tv_nsec) {
    ts1.tv_sec--;
    ts1.tv_nsec += 1'000'000'000;
  }

  ts1.tv_sec -= ts2.tv_sec;
  ts1.tv_nsec -= ts2.tv_nsec;
  return ts1;
}