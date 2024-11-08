#include <stdbool.h>
#include <stdio.h>

int main() {
  int in_cnt = 0;
  char input[512] = {0};
  while (true) {
    char c = getchar();
    printf("%c", c);
    fflush();
    if (c != '\n') {
      input[in_cnt++] = c;
    } else {
      input[in_cnt++] = '\0';
      break;
    }
  }
  printf("%s\n", input);
  return 0;
}
