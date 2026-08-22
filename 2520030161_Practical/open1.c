#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;

    fd = open("A7", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1)
    {
        printf("Error: cannot open file\n");
        return 1;
    }

    printf("File successfully opened in write mode\n");
    printf("File descriptor: %d\n", fd);

    write(fd, "Hello", 5);

    printf("Data written successfully\n");

    close(fd);

    return 0;
}
