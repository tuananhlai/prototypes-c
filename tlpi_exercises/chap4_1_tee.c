#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
  char buf[BUF_SIZE];
  ssize_t num_read;
  size_t num_out_fds = 1;
  int out_fds[num_out_fds];
  out_fds[0] = STDOUT_FILENO;
  for (int i = 1; i < argc; i++) {
    out_fds[i] = open(argv[i], O_WRONLY | O_CREAT, 0755);
    if (out_fds[i] == -1) {
      return 1;
    }
  }

  while (true) {
    num_read = read(STDIN_FILENO, buf, BUF_SIZE);
    if (num_read == 0) {
      break;
    }

    for (size_t i = 0; i < num_out_fds; i++) {
      write(out_fds[i], buf, num_read);
    }
  }
  for (size_t i = 1; i < num_out_fds; i++) {
    close(out_fds[i]);
  }
  return 0;
}