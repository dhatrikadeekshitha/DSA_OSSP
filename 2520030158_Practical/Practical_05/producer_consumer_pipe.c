#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

int main()
{
    int pipefd[2];
    pid_t pid;
    char message[100];
    struct timespec start, end;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        close(pipefd[1]);

        clock_gettime(CLOCK_MONOTONIC, &start);

        ssize_t n = read(pipefd[0], message, sizeof(message) - 1);

        clock_gettime(CLOCK_MONOTONIC, &end);

        if (n > 0)
        {
            message[n] = '\0';

            printf("Child received: %s\n", message);

            double time_taken =
                (end.tv_sec - start.tv_sec) +
                (end.tv_nsec - start.tv_nsec) / 1e9;

            printf("Communication receive time: %.9f seconds\n",
                   time_taken);
        }

        close(pipefd[0]);
    }
    else
    {
        close(pipefd[0]);

        strcpy(message, "Data sent from parent to child through pipe.");

        clock_gettime(CLOCK_MONOTONIC, &start);

        write(pipefd[1], message, strlen(message) + 1);

        clock_gettime(CLOCK_MONOTONIC, &end);

        double time_taken =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Parent sent: %s\n", message);
        printf("Communication send time: %.9f seconds\n",
               time_taken);

        close(pipefd[1]);

        wait(NULL);
    }

    return 0;
}
