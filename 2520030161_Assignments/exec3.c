#include <stdio.h>
#include <unistd.h>

int main()
{
    char *args[] = {"ls", "-l", "-F", NULL};

    execvp("ls", args);

    perror("execvp failed");

    return 1;
}
