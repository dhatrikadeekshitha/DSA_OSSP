#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_INPUT 512
#define MAX_ARGS 64
#define MAX_PATH_LENGTH 4096

/* =========================================
   PARSE COMMAND INTO ARGUMENTS
   ========================================= */

int parse_command(const char *input, char *args[])
{
    char *copy = malloc(strlen(input) + 1);

    if (copy == NULL)
    {
        perror("malloc");
        return -1;
    }

    strcpy(copy, input);

    int argc = 0;

    char *token = strtok(copy, " \t");

    while (token != NULL && argc < MAX_ARGS - 1)
    {
        args[argc] = malloc(strlen(token) + 1);

        if (args[argc] == NULL)
        {
            perror("malloc");

            for (int i = 0; i < argc; i++)
            {
                free(args[i]);
            }

            free(copy);
            return -1;
        }

        strcpy(args[argc], token);

        argc++;

        token = strtok(NULL, " \t");
    }

    args[argc] = NULL;

    free(copy);

    return argc;
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
   DISPLAY PATH
   ========================================= */

void display_path()
{
    const char *path = getenv("PATH");

    printf("\n========== PATH VARIABLE ==========\n");

    if (path == NULL)
    {
        printf("PATH variable is not set.\n");
    }
    else
    {
        printf("%s\n", path);
    }

    printf("===================================\n");
}

/* =========================================
   PARSE AND DISPLAY PATH DIRECTORIES
   ========================================= */

void display_path_directories()
{
    const char *path = getenv("PATH");

    if (path == NULL)
    {
        printf("PATH variable is not set.\n");
        return;
    }

    char *path_copy = malloc(strlen(path) + 1);

    if (path_copy == NULL)
    {
        perror("malloc");
        return;
    }

    strcpy(path_copy, path);

    printf("\n======= PATH DIRECTORIES =======\n");

    int number = 1;

    char *directory = strtok(path_copy, ":");

    while (directory != NULL)
    {
        printf("[%d] %s\n", number, directory);

        number++;

        directory = strtok(NULL, ":");
    }

    printf("===============================\n");

    free(path_copy);
}

/* =========================================
   CHECK EXECUTABLE PERMISSIONS
   ========================================= */

int verify_executable(const char *path)
{
    struct stat file_info;

    /*
     * Check that the path exists
     * and retrieve file information.
     */
    if (stat(path, &file_info) != 0)
    {
        return 0;
    }

    /*
     * Make sure it is a regular file.
     */
    if (!S_ISREG(file_info.st_mode))
    {
        return 0;
    }

    /*
     * Check execute permission.
     */
    if (access(path, X_OK) == 0)
    {
        return 1;
    }

    return 0;
}

/* =========================================
   RESOLVE COMMAND USING PATH
   ========================================= */

char *resolve_command(const char *command)
{
    /*
     * If command already contains '/',
     * do not search PATH.
     */
    if (strchr(command, '/') != NULL)
    {
        if (verify_executable(command))
        {
            char *result =
                malloc(strlen(command) + 1);

            if (result == NULL)
            {
                perror("malloc");
                return NULL;
            }

            strcpy(result, command);

            return result;
        }

        return NULL;
    }

    const char *path = getenv("PATH");

    if (path == NULL)
    {
        return NULL;
    }

    char *path_copy =
        malloc(strlen(path) + 1);

    if (path_copy == NULL)
    {
        perror("malloc");
        return NULL;
    }

    strcpy(path_copy, path);

    char *directory =
        strtok(path_copy, ":");

    while (directory != NULL)
    {
        size_t required_length =
            strlen(directory) +
            strlen(command) +
            2;

        char *candidate =
            malloc(required_length);

        if (candidate == NULL)
        {
            perror("malloc");
            free(path_copy);
            return NULL;
        }

        snprintf(candidate,
                 required_length,
                 "%s/%s",
                 directory,
                 command);

        printf("Checking: %s\n", candidate);

        if (verify_executable(candidate))
        {
            free(path_copy);
            return candidate;
        }

        free(candidate);

        directory = strtok(NULL, ":");
    }

    free(path_copy);

    return NULL;
}

/* =========================================
   DISPLAY RESOLUTION RESULT
   ========================================= */

char *find_executable(const char *command)
{
    printf("\n======= COMMAND RESOLUTION =======\n");

    printf("Command: %s\n", command);

    char *resolved =
        resolve_command(command);

    if (resolved == NULL)
    {
        printf("\nCommand not found or not executable.\n");
        printf("==================================\n");
        return NULL;
    }

    printf("\nExecutable found:\n");
    printf("%s\n", resolved);

    printf("Execute permission: VERIFIED\n");

    printf("==================================\n");

    return resolved;
}

/* =========================================
   CREATE AND MONITOR CHILD PROCESS
   ========================================= */

void execute_command(char *args[],
                     int argc,
                     const char *executable)
{
    if (argc == 0)
    {
        return;
    }

    printf("\n========== PROCESS CONTROL ==========\n");

    pid_t pid = fork();

    /*
     * fork() failed
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
        printf("Child PID  : %d\n",
               getpid());

        printf("Parent PID : %d\n",
               getppid());

        printf("Executing  : %s\n",
               executable);

        /*
         * Execute the resolved executable.
         */
        execv(executable, args);

        /*
         * Reached only if execv fails.
         */
        fprintf(stderr,
                "Child execution error: %s\n",
                strerror(errno));

        _exit(127);
    }

    /*
     * Parent process
     */
    else
    {
        int status;

        printf("Parent PID : %d\n",
               getpid());

        printf("Child PID  : %d\n",
               pid);

        printf("Parent waiting using waitpid()...\n");

        pid_t result =
            waitpid(pid, &status, 0);

        if (result == -1)
        {
            perror("waitpid");
            printf("====================================\n");
            return;
        }

        /*
         * Normal child termination.
         */
        if (WIFEXITED(status))
        {
            int exit_status =
                WEXITSTATUS(status);

            printf("Child terminated normally.\n");
            printf("Exit status: %d\n",
                   exit_status);
        }

        /*
         * Child terminated by a signal.
         */
        else if (WIFSIGNALED(status))
        {
            printf("Child terminated by signal.\n");
            printf("Signal number: %d\n",
                   WTERMSIG(status));
        }

        /*
         * Child was stopped.
         */
        else if (WIFSTOPPED(status))
        {
            printf("Child stopped by signal.\n");
            printf("Signal number: %d\n",
                   WSTOPSIG(status));
        }
    }

    printf("Parent process continues.\n");
    printf("====================================\n");
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[MAX_INPUT];

    printf("========================================\n");
    printf("       SKILL_09 PATH RESOLVER\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Retrieve PATH variable\n");
    printf("- Parse PATH directories\n");
    printf("- Search for executables\n");
    printf("- Verify execute permissions\n");
    printf("- Handle missing commands\n");
    printf("- Create child processes\n");
    printf("- Execute resolved programs\n");
    printf("- Synchronize using waitpid()\n");
    printf("- Monitor child process status\n");

    printf("\nBuilt-in commands:\n");
    printf("path      - Display PATH variable\n");
    printf("dirs      - Display PATH directories\n");
    printf("resolve X - Resolve command X\n");
    printf("exit      - Exit program\n");

    printf("\nOther commands are executed as programs.\n");

    while (1)
    {
        printf("\nskill09> ");
        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /*
         * Empty input
         */
        if (strlen(input) == 0)
        {
            printf("Empty command.\n");
            continue;
        }

        /*
         * Exit
         */
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting Skill_09...\n");
            break;
        }

        /*
         * Display PATH
         */
        if (strcmp(input, "path") == 0)
        {
            display_path();
            continue;
        }

        /*
         * Display PATH directories
         */
        if (strcmp(input, "dirs") == 0)
        {
            display_path_directories();
            continue;
        }

        /*
         * Parse command.
         */
        char *args[MAX_ARGS];

        int argc =
            parse_command(input, args);

        if (argc <= 0)
        {
            continue;
        }

        /*
         * Resolve-only command.
         */
        if (strcmp(args[0], "resolve") == 0)
        {
            if (argc < 2)
            {
                printf("Usage: resolve <command>\n");
            }
            else
            {
                char *resolved =
                    find_executable(args[1]);

                free(resolved);
            }

            free_arguments(args, argc);
            continue;
        }

        /*
         * Resolve command before execution.
         */
        char *executable =
            find_executable(args[0]);

        if (executable == NULL)
        {
            free_arguments(args, argc);
            continue;
        }

        /*
         * Create child and execute command.
         */
        execute_command(args,
                        argc,
                        executable);

        free(executable);

        /*
         * Free command arguments.
         */
        free_arguments(args, argc);
    }

    return 0;
}
