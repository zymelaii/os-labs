#include <assert.h>
#include <random.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
  pid_t pid = fork();

  if (pid == 0) {
    int ret = exec("shell.bin");
    panic("exec failed! errno: %d", ret);
  }

  if (pid < 0) {
    panic("init process fork failed errno: %d", pid);
  }

  int wstatus = 0;
  while (true) {
    pid = wait(&wstatus);
  }

  return 0;
}
