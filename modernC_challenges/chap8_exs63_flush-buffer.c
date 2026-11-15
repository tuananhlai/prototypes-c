#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

void delay(double secs) {
  double const magic = 4e8;
  unsigned long long const nano = secs * magic;
  for (unsigned long volatile count = 0; count < nano; count++) {
  }
}

/**
Observe the behavior of the program by running it with zero, one, and two
command-line arguments.
*/
int main(int argc, [[maybe_unused]] char *argv[argc + 1]) {
  fputs("waiting 10 seconds for you to stop me", stdout);
  if (argc < 3)
    fflush(stdout);

  for (unsigned i = 0; i < 10; i++) {
    fputc('.', stdout);
    if (argc < 2)
      fflush(stdout);
    delay(1.0);
  }
  fputs("\n", stdout);
  fputs("you did ignore me, so bye bye\n", stdout);
  return EXIT_SUCCESS;
}