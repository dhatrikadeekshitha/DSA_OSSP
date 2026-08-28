#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int c1, c2;
    char buf[100];
    int n;

    c1 = open("b1", O_RDWR | O_CREAT, 0644);
    c2 = open("b2", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (c1 == -1 || c2 == -1)
    {
        perror("open");
        return 1;
    }

    printf("Files successfully opened and created\n");
    printf("c1 = %d\n", c1);
    printf("c2 = %d\n", c2);

    n = read(c1, buf, 20);

    if (n > 0)
    {
        write(c2, buf, n);
        write(STDOUT_FILENO, buf, n);
    }

    close(c1);
    close(c2);

    return 0;
}
