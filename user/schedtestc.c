#include "kernel/types.h"
#include "user/user.h"

#define STRIDE_NUM_1 4
#define STRIDE_NUM_2 12

int main(int argc, char* args[]) {
    const int CPU_RUNS = 0x70000000, IO_RUNS = 3;
    int pid1, pid2, i;
    int pipe_pid1_to_parent[2], pipe_parent_to_pid1[2];

    printf("Test 1 (CPU bound):\n");

    pipe(pipe_parent_to_pid1);
    pipe(pipe_pid1_to_parent);

    if ((pid1 = fork()) == 0) {
        close(pipe_parent_to_pid1[1]);
        close(pipe_pid1_to_parent[0]);

        stride(getpid(), STRIDE_NUM_1);

        for (i = 0; i < CPU_RUNS; i++) {
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_1, getruntime(getpid()));

        exit(0);
    }

    if ((pid2 = fork()) == 0) {
        stride(getpid(), STRIDE_NUM_2);

        for (i = 0; i < CPU_RUNS; i++) {
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_2, getruntime(getpid()));

        exit(0);
    }

    close(pipe_parent_to_pid1[0]);
    close(pipe_pid1_to_parent[1]);

    wait(0);
    wait(0);

    printf("Test 2 (IO Bound):\n");

    pid1 = fork();

    if (pid1 == 0) {
        stride(getpid(), STRIDE_NUM_1);

        for (i = 0; i < CPU_RUNS; i++) {
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_1, getruntime(getpid()));

        exit(0);
    }

    pid2 = fork();

    if (pid2 == 0) {
        stride(getpid(), STRIDE_NUM_2);

        for (i = 0; i < IO_RUNS; i++) {
            sleep(1);
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_2, getruntime(getpid()));

        exit(0);
    }

    exit(0);
}
