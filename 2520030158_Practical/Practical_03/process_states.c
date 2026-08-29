#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    printf("Parent process started.\n");
    printf("Parent PID  : %d\n", getpid());
    printf("Parent PPID : %d\n", getppid());

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("\nChild process created.\n");
        printf("Child PID   : %d\n", getpid());
        printf("Child PPID  : %d\n", getppid());

        printf("Child is running...\n");
        sleep(5);

        printf("Child is terminating.\n");
        exit(0);
    }
    else
    {
        printf("\nParent is waiting for child.\n");
        printf("Child PID   : %d\n", pid);

        sleep(2);

        printf("Parent is waiting...\n");
        wait(NULL);

        printf("Child terminated. Parent continues.\n");
    }

    return 0;
}
