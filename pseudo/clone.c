#define _GNU_SOURCE
#include <sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MAX_STACK_SIZE_MB 32 * 1024

static int child_func(void* arg) {
    exit(0);
}

static inline unsigned long long time_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts)) {
        exit(1);
    }
    return ((unsigned long long)ts.tv_sec) * 1000000000LLU +
            (unsigned long long)ts.tv_nsec;
}

int main(int argc, char** argv) {
    //printf("pid = %d\n", getpid());

    //printf("Press ENTER key to Continue\n");  
    getchar();

    bool share_page_table;
    if (!strcmp(argv[1], "share")) {
        share_page_table = true;
    } else if (!strcmp(argv[1], "copy")) {
        share_page_table = false;
    } else {
        printf("Usage: %s <share|copy> <heap mem size in MB>\n", argv[0]);
        exit(0);
    }


    int heap_size_m = atoi(argv[2]);

    if(heap_size_m < 0 || heap_size_m > MAX_STACK_SIZE_MB) {
        printf("Not support heap mem size, %s!\n", argv[2]);
        exit(0);
    }

    int heap_size_b = heap_size_m * 1e6;
    void* heap = NULL;
    if(heap_size_b != 0) {
        // initialize with 0, don't alloc lazily
        heap = calloc(heap_size_b, 1);
        if (!heap) {
            perror("heap calloc error");
            exit(1);
        }
    }

    int stack_size_m = 1;
    int stack_size_b = stack_size_m * 1e6;
    void* stack = malloc(stack_size_b);
    if (!stack) {
        perror("stack malloc error");
        exit(1);
    }
    

    unsigned long flags = 0;
    if (share_page_table) {
        flags |= CLONE_VM;
    }

    int N = 1000;
    unsigned long long max_launch = 0;
    unsigned long long min_launch = ~max_launch;
    unsigned long long total_launch = 0;

    for (int i = 0; i < N; ++i) {
        unsigned long long t1 = time_ns();

        if (clone(child_func, stack == NULL ? NULL : stack + stack_size_b, flags | SIGCHLD, NULL) == -1) {
            perror("clone");
            exit(1);
        }

        unsigned long long elapsed = time_ns() - t1;
        min_launch = elapsed < min_launch ? elapsed : min_launch;
        max_launch = elapsed > max_launch ? elapsed : max_launch;
        total_launch += elapsed;

        int status;
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }

        // printf("Child exited with status %d.\n", status);
        // return 0;
    }

    printf("After %d iterations:\n  min_launch = %llu\n  max_launch = %llu\n", N, min_launch, max_launch);
    printf("Average:\n  launch = %.2lf\n", (double)total_launch / N);
}
