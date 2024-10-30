#include <stdbool.h>
#include <stdio.h>

int main() {
  while (true) {
    char ch = getch();
    if (ch == -1) {
      continue;
    }
    printf("%c", ch);
    fflush();
  }
}
