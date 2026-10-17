
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    const char *fifo = "/tmp/ossp_fifo_2520030216";
    char message[] = "Hello from parent process";
    char buffer[100];
    int fd;
    pid_t pid;

    if (mkfifo(fifo, 0666) == -1 &&
        access(fifo, F_OK) == -1)
    {
        perror("mkfifo failed");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        fd = open(fifo, O_RDONLY);
        if (fd == -1)
        {
            perror("open failed");
            exit(1);
        }

        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        if (n < 0)
        {
            perror("read failed");
            close(fd);
            exit(1);
        }

        buffer[n] = '\0';
        printf("Child received: %s\n", buffer);

        close(fd);
        exit(0);
    }
    else
    {
        fd = open(fifo, O_WRONLY);
        if (fd == -1)
        {
            perror("open failed");
            waitpid(pid, NULL, 0);
            return 1;
        }

        size_t len = strlen(message);
        if (write(fd, message, len) != (ssize_t)len)
        {
            perror("write failed");
            close(fd);
            waitpid(pid, NULL, 0);
            return 1;
        }

        close(fd);
        waitpid(pid, NULL, 0);

        printf("Parent: Message sent successfully.\n");
    }

    return 0;
}
