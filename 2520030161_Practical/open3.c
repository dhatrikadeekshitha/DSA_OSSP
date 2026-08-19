#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int fd1, fd2;
    char buf[100];
    int n;

    fd1 = open("open3.c", O_RDONLY);

    if (fd1 == -1)
    {
        printf("Error opening source file\n");
        return 1;
    }

    fd2 = open("z7", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd2 == -1)
    {
        printf("Error opening destination file\n");
        close(fd1);
        return 1;
    }

    n = read(fd1, buf, sizeof(buf));

    write(fd2, buf, n);

    printf("Files successfully opened and data copied\n");
    printf("Source file descriptor: %d\n", fd1);
    printf("Destination file descriptor: %d\n", fd2);

    close(fd1);
    close(fd2);

    return 0;
}
