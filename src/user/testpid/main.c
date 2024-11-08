#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  while (true) {
    printf("pid: %d!", get_pid());
    fflush();
    for (int i = 0; i < (int)1e8; ++i) {
    }
  }
  return 0;
}
