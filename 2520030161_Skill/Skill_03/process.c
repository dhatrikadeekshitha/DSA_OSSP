#include <stdio.h>
#include <unistd.h>
int main()
{
    printf("Before fork\n");
    fork();
    printf("Process ID = %d\n", getpid());
    return 0;
}
