#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  pid_t pid = fork();
  if (pid != 0) {
    while (true) {
      printf("I'm fa, son pid = %d", pid);
      fflush();
      for (int i = 0; i < (int)1e8; ++i) {
      }
    }
  } else {
    while (true) {
      printf("I'm son");
      fflush();
      for (int i = 0; i < (int)1e8; ++i) {
      }
    }
  }
  return 0;
}
