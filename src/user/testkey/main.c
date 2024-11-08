#include <stdbool.h>
#include <stdio.h>

int main() {
  while (true) {
    char ch = getch();
    if ((int)ch == 0xff) {
      continue;
    }
    printf("%c", ch);
    fflush();
  }
  return 0;
}
