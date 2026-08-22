#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    printf("Parent process started.\n");
    printf("Parent PID: %d\n", getpid());

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        printf("Child process started.\n");
        printf("Child PID: %d\n", getpid());

        printf("Child process is terminating...\n");
        exit(0);
    }
    else
    {
        printf("Parent is waiting for child...\n");

        wait(NULL);

        printf("Child process terminated successfully.\n");
        printf("Parent process is terminating...\n");

        exit(0);
    }

    return 0;
}
