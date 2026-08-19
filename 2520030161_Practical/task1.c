#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int c1, c2;
    char buf[100];
    int n;

    c1 = open("b1", O_RDONLY);

    if (c1 == -1)
    {
        printf("Error opening b1\n");
        return 1;
    }

    c2 = open("b2", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (c2 == -1)
    {
        printf("Error opening b2\n");
        close(c1);
        return 1;
    }

    printf("Files successfully opened\n");
    printf("b1 file descriptor: %d\n", c1);
    printf("b2 file descriptor: %d\n", c2);

    n = read(c1, buf, sizeof(buf));

    write(c2, buf, n);

    printf("Data copied successfully\n");

    close(c1);
    close(c2);

    return 0;
}
