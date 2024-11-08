#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main() {
  int child_status = 0;
  while (true) {
    printf("\x1b[32mMINIOS\x1b[0m$ ");
    fflush();

    int in_cnt = 0;
    char input[512] = {0};
    while (true) {
      char c = getchar();

      if (c == '\n') {
        printf("%c", c);
        fflush();
        input[in_cnt++] = '\0';
        break;
      } else if (c == '\b') {
        if (in_cnt == 0) continue;
        printf("\b \b");
        fflush();
        in_cnt--;
      } else {
        printf("%c", c);
        fflush();
        input[in_cnt++] = c;
      }
    }

    if (strcmp(input, "?") == 0) {
      printf("%d\n", WEXITSTATUS(child_status));
      continue;
    }

    int ForkRetVal = fork();
    if (ForkRetVal < 0) {
      printf("fork failed!\n");
      continue;
    }
    if (ForkRetVal == 0) {
      exec(input);
      printf("exec failed!\n");
      exit(1);
    }

    int wait_pid;
    wait_pid = wait(&child_status);
    if ((wait_pid = wait(&child_status)) >= 0) {
    }
  }
  return 0;
}
