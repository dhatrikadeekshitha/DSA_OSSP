#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    int pid;
    char buf[20] = "Hello";

    pipe(fd);

    pid = fork();

    if (pid > 0)
    {
        close(fd[0]);

        write(fd[1], buf, sizeof(buf));

        close(fd[1]);

        wait(NULL);
    }
    else if (pid == 0)
    {
        close(fd[1]);

        read(fd[0], buf, sizeof(buf));

        printf("Child received: %s from parent\n", buf);

        close(fd[0]);
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
