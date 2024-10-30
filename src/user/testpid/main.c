#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#pragma GCC push_options
#pragma GCC optimize("O0")
void busy_delay() {
  for (int i = 0; i < 1e8; ++i) {
  }
}
#pragma GCC pop_options

int main() {
  while (true) {
    printf("pid: %d!", get_pid());
    fflush();
    busy_delay();
  }
  return 0;
}
