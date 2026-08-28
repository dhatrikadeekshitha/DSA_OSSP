#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define SIZE 1000000

void print_memory_info(const char *process_name)
{
    char path[64];
    char line[256];

    snprintf(path, sizeof(path), "/proc/%d/status", getpid());

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    printf("\n[%s] PID: %d\n", process_name, getpid());

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "VmSize:", 7) == 0 ||
            strncmp(line, "VmRSS:", 6) == 0)
        {
            printf("%s", line);
        }
    }

    fclose(file);
}

int main()
{
    int *data = malloc(SIZE * sizeof(int));

    if (data == NULL)
    {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < SIZE; i++)
    {
        data[i] = i;
    }

    printf("Copy-on-Write Demonstration\n");
    printf("===========================\n");

    print_memory_info("Parent before fork");

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        free(data);
        return 1;
    }

    if (pid == 0)
    {
        print_memory_info("Child after fork");

        printf("\nChild modifying data...\n");

        for (int i = 0; i < SIZE; i++)
        {
            data[i] = data[i] + 100;
        }

        print_memory_info("Child after modification");

        printf("Child data[0] = %d\n", data[0]);

        free(data);
        exit(0);
    }
    else
    {
        sleep(2);

        print_memory_info("Parent while child is running");

        wait(NULL);

        printf("\nChild process completed.\n");

        free(data);
    }

    return 0;
}
