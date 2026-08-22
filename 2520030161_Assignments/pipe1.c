#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    int pid;

    pipe(fd);

    pid = fork();

    if (pid > 0)
    {
        close(fd[0]);

        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        execlp("cat", "cat", "pipe1.c", NULL);
    }
    else if (pid == 0)
    {
        close(fd[1]);

        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        execlp("wc", "wc", "-l", "-c", NULL);
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
