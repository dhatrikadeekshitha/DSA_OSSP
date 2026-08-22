#include <stdio.h>
#include <unistd.h>

int main()
{
    int fd[2];

    if (pipe(fd) == -1)
    {
        printf("Pipe creation failed\n");
        return 1;
    }

    printf("Pipe created successfully\n");
    printf("Read end of pipe: %d\n", fd[0]);
    printf("Write end of pipe: %d\n", fd[1]);

    close(fd[0]);
    close(fd[1]);

    return 0;
}
