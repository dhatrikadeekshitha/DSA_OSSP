#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pid = fork();

    if (pid == 0)
    {
        printf("Hello from child process\n");
    }
    else if (pid > 0)
    {
        printf("Hello from parent process\n");
        wait(NULL);
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
