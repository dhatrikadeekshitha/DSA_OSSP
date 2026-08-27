#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_INPUT 512
#define MAX_ARGS 64

/* =========================================
   TOKENIZATION WITH ESCAPE SEQUENCES
   ========================================= */

int parse_command(const char *input, char *args[])
{
    int argc = 0;
    int i = 0;

    char *token = malloc(MAX_INPUT);

    if (token == NULL)
    {
        perror("malloc");
        return -1;
    }

    int token_length = 0;
    int in_token = 0;

    while (input[i] != '\0')
    {
        char ch = input[i];

        /*
         * Backslash escape sequence
         *
         * Example:
         * hello\ world
         *
         * becomes:
         * hello world
         */

        if (ch == '\\')
        {
            i++;

            if (input[i] == '\0')
            {
                printf("\nParser Error: Backslash at end of input.\n");
                free(token);
                return -1;
            }

            /*
             * Preserve the character after
             * the backslash literally.
             */
            token[token_length++] = input[i];
            in_token = 1;

            i++;
            continue;
        }

        /*
         * Whitespace outside an escaped sequence
         * separates tokens.
         */
        if (isspace((unsigned char)ch))
        {
            if (in_token)
            {
                token[token_length] = '\0';

                if (argc >= MAX_ARGS - 1)
                {
                    printf("Parser Error: Too many arguments.\n");
                    free(token);
                    return -1;
                }

                args[argc] = malloc(strlen(token) + 1);

                if (args[argc] == NULL)
                {
                    perror("malloc");
                    free(token);
                    return -1;
                }

                strcpy(args[argc], token);
                argc++;

                token_length = 0;
                in_token = 0;
            }

            i++;
            continue;
        }

        /*
         * Normal character.
         */
        token[token_length++] = ch;
        in_token = 1;

        i++;
    }

    /*
     * Store final token.
     */
    if (in_token)
    {
        token[token_length] = '\0';

        if (argc >= MAX_ARGS - 1)
        {
            printf("Parser Error: Too many arguments.\n");
            free(token);
            return -1;
        }

        args[argc] = malloc(strlen(token) + 1);

        if (args[argc] == NULL)
        {
            perror("malloc");
            free(token);
            return -1;
        }

        strcpy(args[argc], token);
        argc++;
    }

    args[argc] = NULL;

    free(token);

    return argc;
}

/* =========================================
   DISPLAY PARSER OUTPUT
   ========================================= */

void display_parser_output(char *args[], int argc)
{
    printf("\n========== PARSER OUTPUT ==========\n");

    printf("Argument count: %d\n", argc);

    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = \"%s\"\n", i, args[i]);
    }

    printf("===================================\n");
}

/* =========================================
   FREE ARGUMENTS
   ========================================= */

void free_arguments(char *args[], int argc)
{
    for (int i = 0; i < argc; i++)
    {
        free(args[i]);
        args[i] = NULL;
    }
}

/* =========================================
   EXECUTE COMMAND
   ========================================= */

void execute_command(char *args[], int argc)
{
    if (argc == 0)
    {
        printf("No command to execute.\n");
        return;
    }

    printf("\nLaunching command: %s\n",
           args[0]);

    pid_t pid = fork();

    /*
     * fork() error
     */
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    /*
     * Child process
     */
    if (pid == 0)
    {
        printf("Child process PID: %d\n",
               getpid());

        /*
         * Replace child process with
         * requested program.
         */
        execvp(args[0], args);

        /*
         * execvp() only returns when
         * execution fails.
         */
        fprintf(stderr,
                "Execution Error: %s: %s\n",
                args[0],
                strerror(errno));

        exit(EXIT_FAILURE);
    }

    /*
     * Parent process
     */
    else
    {
        int status;

        printf("Parent process PID: %d\n",
               getpid());

        printf("Waiting for child process...\n");

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return;
        }

        if (WIFEXITED(status))
        {
            printf("Child exited with status: %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child terminated by signal: %d\n",
                   WTERMSIG(status));
        }
    }
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[MAX_INPUT];

    printf("========================================\n");
    printf("      SKILL_08 PROCESS LAUNCHER\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Escape sequence processing\n");
    printf("- Escaped spaces\n");
    printf("- Escaped special symbols\n");
    printf("- Character preservation\n");
    printf("- Command parsing\n");
    printf("- Child process creation\n");
    printf("- Program execution using execvp()\n");
    printf("- Argument passing\n");
    printf("- Parent process management\n");

    printf("\nExamples:\n");
    printf("  echo hello\n");
    printf("  echo hello\\ world\n");
    printf("  echo hello\\|world\n");
    printf("  ls -l\n");
    printf("  pwd\n");

    printf("\nType 'exit' to quit.\n");

    while (1)
    {
        printf("\nskill08> ");
        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /*
         * Handle empty input.
         */
        if (strlen(input) == 0)
        {
            printf("Empty command.\n");
            continue;
        }

        /*
         * Exit condition.
         */
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting Skill_08...\n");
            break;
        }

        char *args[MAX_ARGS];

        int argc =
            parse_command(input, args);

        if (argc < 0)
        {
            continue;
        }

        /*
         * Display parser result before
         * executing the command.
         */
        display_parser_output(args, argc);

        /*
         * Execute parsed command.
         */
        execute_command(args, argc);

        /*
         * Release dynamically allocated
         * argument memory.
         */
        free_arguments(args, argc);
    }

    return 0;
}
