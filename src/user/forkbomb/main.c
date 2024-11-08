#include <random.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void bomb_salve() {
  fork_ack();

  for (int i = 0; i < (get_pid() & 0xff); i++) rand();
  for (int i = (rand() & 0xf); i > 0; --i) {
    int pid = fork();
    if (pid == 0) {
      bomb_salve();
    }
    for (int i = rand(); i > 0; --i) {
    }
  }

  if (rand() % 2 == 1) {
    while (wait(NULL) >= 0) {
    }
  }

  exit(0);
}

void bomb_master() {
  while (true) {
    int pid = fork();
    if (pid == 0) {
      bomb_salve();
    }
    for (int i = rand(); i > 0; --i) {
    }
    while (wait(NULL) >= 0) {
    }
  }
}

int main() {
  srand(get_ticks());
  bomb_master();
  return 0;
}
