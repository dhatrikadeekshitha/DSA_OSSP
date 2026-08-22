#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
        printf("Child process running.\n");
        printf("Child PID  = %d\n", getpid());
        printf("Parent PID = %d\n", getppid());

        printf("Child terminating now...\n");
        exit(0);
    }
    else
    {
        printf("Parent process running.\n");
        printf("Parent PID = %d\n", getpid());
        printf("Child PID  = %d\n", pid);

        printf("Parent sleeping for 20 seconds without wait().\n");
        printf("During this time the child becomes a zombie.\n");

        sleep(20);

        printf("Parent terminating.\n");
    }

    return 0;
}

