#include <stdio.h>
#include <unistd.h>

int main()
{
    execl("/bin/pwd", "pwd", NULL);

    perror("execl failed");

    return 1;
}
