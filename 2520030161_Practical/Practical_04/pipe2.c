#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd[2];
    pipe(fd);

    int pid = fork();

    if (pid > 0)
    {
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);

        execl("/bin/ls", "ls", "-l", NULL);
    }

    if (pid == 0)
    {
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);

        execlp("grep", "grep", "grep.c", NULL);
    }

    return 0;
}




