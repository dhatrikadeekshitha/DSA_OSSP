#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child exiting. PID = %d\n", getpid());
        exit(0);
    }
    else
    {
        printf("Parent PID = %d\n", getpid());
        printf("Child PID  = %d\n", pid);

        printf("Parent sleeping. Check the child process using:\n");
        printf("ps -el | grep Z\n");

        sleep(20);

        printf("Parent exiting.\n");
    }

    return 0;
}
