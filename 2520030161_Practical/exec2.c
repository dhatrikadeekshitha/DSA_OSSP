#include <stdio.h>
#include <unistd.h>

int main()
{
    execlp("pwd", "pwd", NULL);

    perror("execlp failed");

    return 1;
}
