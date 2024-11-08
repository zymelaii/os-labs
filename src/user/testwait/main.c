#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/wait.h>

void test_fork_wait1() {
  ssize_t pid = fork();

  assert(pid >= 0);

  if (pid == 0) exit(114);

  int wstatus;
  assert(pid == wait(&wstatus));

  assert(WEXITSTATUS(wstatus) == 114);

  printf("\x1b[92mtest_fork_wait1 passed!\x1b[0m\n");
}

void test_fork_wait2() {
  ssize_t pid = fork();

  assert(pid >= 0);

  if (pid == 0) exit(514);

  assert(pid == wait(NULL));

  printf("\x1b[92mtest_fork_wait2 passed!\x1b[0m\n");
}

void test_empty_wait() {
  assert(wait(NULL) == -ECHILD);

  printf("\x1b[92mtest_empty_wait passed!\x1b[0m\n");
}

void test_fork_limit() {
  int xor_sum = 0;

  for (int i = 1; i <= 17; i++) {
    pid_t pid = fork();

    assert(pid >= 0);

    if (pid == 0) exit(0);

    xor_sum ^= pid;
  }

  assert(fork() == -EAGAIN);

  int wait_cnt = 0, wait_pid;
  while ((wait_pid = wait(NULL)) >= 0) wait_cnt++, xor_sum ^= wait_pid;

  assert(wait_cnt == 17);
  assert(xor_sum == 0);

  printf("\x1b[92mtest_fork_limit passed!\x1b[0m\n");
}

void test_wait_is_sleeping() {
  pid_t rt_pid = get_pid();

  for (int i = 1; i <= 17; i++) {
    pid_t pid = fork();

    assert(pid >= 0);

    if (pid > 0) {
      int wstatus;
      assert(pid == wait(&wstatus));
      assert(WEXITSTATUS(wstatus) == 42);
      if (get_pid() != rt_pid) exit(42);
      break;
    }
  }

  if (get_pid() != rt_pid) {
    ssize_t la_ticks = get_ticks();

    for (int i = 1; i <= (int)5e6; i++) {
      ssize_t now_ticks = get_ticks();
      assert(now_ticks - la_ticks <= 2);
      la_ticks = now_ticks;
    }

    exit(42);
  }

  printf("\x1b[92mtest_wait_is_sleeping passed!\x1b[0m\n");
}

int main() {
  test_fork_wait1();
  test_fork_wait2();
  test_empty_wait();
  test_fork_limit();
  test_wait_is_sleeping();
  printf("\x1b[92mall tests passed!\x1b[0m\n");
  return 0;
}
