#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    int fd[2];
    char buf[10] = "hello";

    // Create pipe before fork
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    int pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid > 0)
    {
        // Parent process
        close(fd[0]);

        write(fd[1], buf, sizeof(buf));

        close(fd[1]);
    }
    else
    {
        // Child process
        close(fd[1]);

        read(fd[0], buf, sizeof(buf));

        printf("Child received %s from parent\n", buf);

        close(fd[0]);
    }

    return 0;
}
