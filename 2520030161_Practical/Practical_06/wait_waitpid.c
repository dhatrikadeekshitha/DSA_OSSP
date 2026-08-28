#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t pid[3];
    int status;

    printf("Parent PID: %d\n\n", getpid());

    /* Create three child processes */
    for (int i = 0; i < 3; i++)
    {
        pid[i] = fork();

        if (pid[i] < 0)
        {
            perror("fork failed");
            return 1;
        }

        if (pid[i] == 0)
        {
            printf("Child %d started. PID = %d, PPID = %d\n",
                   i + 1, getpid(), getppid());

            sleep(2 + i);

            printf("Child %d terminating. PID = %d\n",
                   i + 1, getpid());

            exit(10 + i);
        }
    }

    printf("\nParent waiting using wait()...\n");

    /*
     * wait() waits for any one child to terminate.
     */
    pid_t finished = wait(&status);

    if (finished > 0)
    {
        printf("wait(): Child PID %d completed\n", finished);

        if (WIFEXITED(status))
            printf("Exit status = %d\n", WEXITSTATUS(status));
    }

    printf("\nParent waiting using waitpid()...\n");

    /*
     * waitpid() waits for a specific child.
     */
    for (int i = 0; i < 3; i++)
    {
        if (pid[i] != finished)
        {
            pid_t result = waitpid(pid[i], &status, 0);

            if (result > 0)
            {
                printf("waitpid(): Child PID %d completed\n", result);

                if (WIFEXITED(status))
                    printf("Exit status = %d\n", WEXITSTATUS(status));
            }
        }
    }

    printf("\nAll child processes have completed.\n");
    printf("Parent process terminating.\n");

    return 0;
}
