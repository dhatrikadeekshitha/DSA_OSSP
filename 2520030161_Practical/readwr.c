#include <stdio.h>
#include <unistd.h>

int main()
{
    char buf[100];
    int n;

    printf("Enter text: ");

    n = read(STDIN_FILENO, buf, sizeof(buf));

    write(STDOUT_FILENO, buf, n);

    return 0;
}
