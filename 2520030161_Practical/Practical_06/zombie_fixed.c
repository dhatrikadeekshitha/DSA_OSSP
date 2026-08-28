#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process running.\n");
        printf("Child PID  = %d\n", getpid());
        printf("Parent PID = %d\n", getppid());

        printf("Child terminating now...\n");
        exit(0);
    }
    else
    {
        printf("Parent process running.\n");
        printf("Parent PID = %d\n", getpid());
        printf("Child PID  = %d\n", pid);

        printf("Parent waiting for child using waitpid().\n");

        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("Child terminated normally.\n");
            printf("Child exit status = %d\n", WEXITSTATUS(status));
        }

        printf("Zombie eliminated because parent collected child status.\n");
        printf("Parent terminating.\n");
    }

    return 0;
}
