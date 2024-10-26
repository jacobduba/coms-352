#include "kernel/types.h"
#include "user/user.h"

#define STRIDE_NUM_1 4
#define STRIDE_NUM_2 12

int main(int argc, char* args[]) {
    int pid1 = fork();
    const int RUNS = 0x50000000;

    if (pid1 == 0) {
        stride(getpid(), STRIDE_NUM_1);

        for (int i = 0; i < RUNS; i++) {
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_1, getruntime(getpid()));

        exit(0);
    }

    int pid2 = fork();

    if (pid2 == 0) {
        stride(getpid(), STRIDE_NUM_2);

        for (int i = 0; i < RUNS; i++) {
        }

        printf("Stride %d process: runtime of %d ticks.\n", STRIDE_NUM_2, getruntime(getpid()));

        exit(0);
    }

    wait(0);
    wait(0);

    exit(0);
}
