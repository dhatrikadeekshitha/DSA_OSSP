#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t child1, child2, child3;
    int status;

    child1 = fork();

    if (child1 == 0)
    {
        printf("Child 1: PID = %d\n", getpid());
        sleep(3);
        exit(10);
    }

    child2 = fork();

    if (child2 == 0)
    {
        printf("Child 2: PID = %d\n", getpid());
        sleep(1);
        exit(20);
    }

    child3 = fork();

    if (child3 == 0)
    {
        printf("Child 3: PID = %d\n", getpid());
        sleep(2);
        exit(30);
    }

    printf("Parent PID = %d\n", getpid());

    printf("\nUsing wait():\n");

    pid_t finished = wait(&status);

    printf("wait() returned child PID = %d\n", finished);

    if (WIFEXITED(status))
        printf("Exit status = %d\n", WEXITSTATUS(status));

    printf("\nUsing waitpid():\n");

    waitpid(child1, &status, 0);

    printf("waitpid() waited specifically for Child 1: PID = %d\n", child1);

    if (WIFEXITED(status))
        printf("Child 1 exit status = %d\n", WEXITSTATUS(status));

    waitpid(child2, NULL, 0);
    waitpid(child3, NULL, 0);

    printf("All children completed.\n");

    return 0;
}
