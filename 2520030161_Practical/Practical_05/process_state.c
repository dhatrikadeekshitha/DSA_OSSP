#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

void print_info(const char *stage)
{
    printf("\n[%s]\n", stage);
    printf("PID  : %d\n", getpid());
    printf("PPID : %d\n", getppid());
    fflush(stdout);
}

int main()
{
    pid_t pid;

    printf("Parent process starting...\n");
    printf("Initial PID  : %d\n", getpid());
    printf("Initial PPID : %d\n", getppid());
    fflush(stdout);

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        /* Child process */

        print_info("CHILD - Running");

        printf("Child entering waiting/sleeping state for 10 seconds...\n");
        fflush(stdout);
        sleep(10);

        print_info("CHILD - Ready/Running");

        printf("Child performing CPU work for 10 seconds...\n");
        fflush(stdout);

        volatile unsigned long long i;
        for (i = 0; i < 5000000000ULL; i++)
        {
            if (i % 1000000000ULL == 0)
            {
                printf("Child CPU work: %llu\n", i);
                fflush(stdout);
            }
        }

        print_info("CHILD - Before Termination");

        printf("Child terminating...\n");
        fflush(stdout);

        exit(0);
    }
    else
    {
        /* Parent process */

        printf("\n[PARENT - Running]\n");
        printf("Parent PID  : %d\n", getpid());
        printf("Child PID   : %d\n", pid);
        printf("Parent PPID : %d\n", getppid());
        fflush(stdout);

        printf("\nParent will wait for child to complete.\n");
        printf("Parent entering WAITING state...\n");
        fflush(stdout);

        waitpid(pid, NULL, 0);

        print_info("PARENT - Child Terminated");

        printf("Parent resumed after child termination.\n");
        fflush(stdout);

        printf("Parent terminating...\n");

        return 0;
    }
}
