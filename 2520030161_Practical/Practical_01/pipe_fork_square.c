#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
    int fd[2];
    int num;

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("Enter a number: ");
    scanf("%d", &num);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid > 0)
    {
        // Parent process
        close(fd[0]);

        write(fd[1], &num, sizeof(num));

        close(fd[1]);
    }
    else
    {
        // Child process
        int received_num;

        close(fd[1]);

        read(fd[0], &received_num, sizeof(received_num));

        printf("Child received number: %d\n", received_num);
        printf("Square of %d = %d\n",
               received_num, received_num * received_num);

        close(fd[0]);
    }

    return 0;
}
