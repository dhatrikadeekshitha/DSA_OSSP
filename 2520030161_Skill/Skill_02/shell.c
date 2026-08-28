#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

int main()
{
    char input[MAX_INPUT];

    printf("Simple Shell Started\n");

    while (1)
    {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT, stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting shell...\n");
            break;
        }

        printf("Command entered: %s\n", input);
    }

    return 0;
}
