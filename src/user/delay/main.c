#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int pid = get_pid();
  int target_ticks = (pid + 1) * 1000;

  while (true) {
    printf("\x1b[%dmI'm %d!\x1b[0m", 90 + pid + 1, pid);
    fflush();
    int before_ticks = get_ticks();
    delay_ticks(target_ticks);
    int after_ticks = get_ticks();

    int real_ticks = after_ticks - before_ticks;
    int delta_ticks = real_ticks - target_ticks;
    assert(-2 <= delta_ticks && delta_ticks <= 2);
  }
}
