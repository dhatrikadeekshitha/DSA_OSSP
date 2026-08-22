#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pid;

    pid = fork();

    if (pid == 0)
    {
        printf("Child process\n");
        printf("Child process ID: %d\n", getpid());
        printf("Parent process ID: %d\n", getppid());
    }
    else if (pid > 0)
    {
        printf("Parent process\n");
        printf("Parent process ID: %d\n", getpid());
        wait(NULL);
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
