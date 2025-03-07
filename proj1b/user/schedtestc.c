#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/proc_queue.h"
#include "user/user.h"

#define CPU_RUNS 2147483647
#define IO_RUNS 10
#define STRIDE_NUM_1 4
#define STRIDE_NUM_2 12

// I wanted to double check my queue data structure
void queuetest() {
  printf("Test 0 (Process queue tests):\n");
  process_queue_init();
  enqueue(2);
  enqueue(1);
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  enqueue(0);
  enqueue(5);
  enqueue(8);
  enqueue(19);
  enqueue(2);
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  insert(1, 1);
  insert(4, 4);
  insert(0, 0);
  insert(10, 10);
  insert(5, 5);
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  insert(1, 1);
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
  printf("%d\n", dequeue());
}

void cpu_bound_test() {
  int pid1, pid2, i;

  printf("Test 1 (CPU bound):\n");

  pid1 = fork();
  if (pid1 == 0) {
    stride(getpid(), STRIDE_NUM_1);

    for (i = 0; i < CPU_RUNS; i++) {
      if (i % (CPU_RUNS / 5) == 0) {
        printf("Stride %d process: runtime reached %d ticks.\n", STRIDE_NUM_1,
               getruntime(getpid()));
      }
    }

    exit(0);
  }

  if ((pid2 = fork()) == 0) {
    stride(getpid(), STRIDE_NUM_2);

    for (i = 0; i < CPU_RUNS; i++) {
      if (i % (CPU_RUNS / 5) == 0) {
        printf("Stride %d process: runtime reached %d ticks.\n", STRIDE_NUM_2,
               getruntime(getpid()));
      }
    }
    exit(0);
  }

  wait(0);
  wait(0);
}

void io_bound_test() {
  int pid1, pid2, i;

  printf("Test 2 (IO Bound):\n");

  pid1 = fork();

  if (pid1 == 0) {
    stride(getpid(), STRIDE_NUM_1);

    for (i = 0; i < CPU_RUNS; i++) {
    }

    printf("Stride %d process: runtime ended at %d ticks.\n", STRIDE_NUM_1,
           getruntime(getpid()));

    exit(0);
  }

  pid2 = fork();

  if (pid2 == 0) {
    stride(getpid(), STRIDE_NUM_2);

    for (i = 0; i < IO_RUNS; i++) {
      sleep(1);
    }

    printf("Stride %d process: runtime ended at %d ticks.\n", STRIDE_NUM_2,
           getruntime(getpid()));

    exit(0);
  }

  wait(0);
  wait(0);
}

int main(int argc, char *args[]) {
  printf("===============================\n");
  if (SCHEDULER == 2) {
    printf("Using round robin scheduler\n");
  } else if (SCHEDULER == 3) {
    printf("Using stride scheduler\n");
  } else if (SCHEDULER == 1) {
    printf("Using default scheduler...:\n");
    printf("Why are you testing this lol?");
  }
  printf("===============================\n");

  // queuetest();
  cpu_bound_test();
  io_bound_test();
  exit(0);
}
