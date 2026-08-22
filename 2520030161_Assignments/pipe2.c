#include <stdio.h>
#include <unistd.h>

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

        execl("/bin/ls", "ls", "-l", NULL);
    }
    else if (pid == 0)
    {
        close(fd[1]);

        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        execlp("grep", "grep", "grep.c", NULL);
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
