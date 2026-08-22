#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pipefd[2];
    pid_t pid;

    pipe(pipefd);

    pid = fork();

    if (pid < 0)
    {
        printf("Fork failed\n");
        return 1;
    }

    if (pid == 0)
    {
        // Child process: cat pipeex.c
        dup2(pipefd[1], STDOUT_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);

        execlp("cat", "cat", "pipeex.c", NULL);

        printf("cat failed\n");
        exit(1);
    }
    else
    {
        // Parent process: sort
        dup2(pipefd[0], STDIN_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);

        execlp("sort", "sort", NULL);

        printf("sort failed\n");
        wait(NULL);
    }

    return 0;
}
