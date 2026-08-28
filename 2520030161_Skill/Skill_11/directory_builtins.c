#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define MAX_INPUT 512
#define MAX_ARGS 32
#define MAX_PATH 4096

/* =========================================
   GLOBAL SHELL STATE
   ========================================= */

char current_directory[MAX_PATH];
char previous_directory[MAX_PATH];

/* =========================================
   DUPLICATE STRING
   ========================================= */

char *duplicate_string(const char *source)
{
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, source);

    return copy;
}

/* =========================================
   PARSE COMMAND
   ========================================= */

int parse_command(const char *input, char *args[])
{
    char *copy = duplicate_string(input);

    int argc = 0;

    char *token = strtok(copy, " \t");

    while (token != NULL && argc < MAX_ARGS - 1)
    {
        args[argc] = duplicate_string(token);

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
   UPDATE CURRENT DIRECTORY
   ========================================= */

int update_current_directory()
{
    if (getcwd(current_directory,
               sizeof(current_directory)) == NULL)
    {
        perror("getcwd");
        return 0;
    }

    return 1;
}

/* =========================================
   DISPLAY CURRENT DIRECTORY
   ========================================= */

int builtin_pwd(char *args[], int argc)
{
    (void)args;
    (void)argc;

    if (!update_current_directory())
    {
        return 1;
    }

    printf("%s\n", current_directory);

    return 0;
}

/* =========================================
   VALIDATE DIRECTORY
   ========================================= */

int validate_directory(const char *path)
{
    if (path == NULL || strlen(path) == 0)
    {
        return 0;
    }

    /*
     * access() checks whether the path
     * exists and can be accessed.
     */
    if (access(path, F_OK) != 0)
    {
        return 0;
    }

    /*
     * Check whether the path is a directory.
     */
    if (chdir(path) != 0)
    {
        return 0;
    }

    return 1;
}

/* =========================================
   CHANGE DIRECTORY
   ========================================= */

int builtin_cd(char *args[], int argc)
{
    char target[MAX_PATH];

    /*
     * cd with no argument -> HOME
     */
    if (argc < 2)
    {
        const char *home = getenv("HOME");

        if (home == NULL)
        {
            printf("cd: HOME is not set.\n");
            return 1;
        }

        strncpy(target,
                home,
                sizeof(target) - 1);

        target[sizeof(target) - 1] = '\0';
    }

    /*
     * cd - -> previous directory
     */
    else if (strcmp(args[1], "-") == 0)
    {
        if (strlen(previous_directory) == 0)
        {
            printf("cd: previous directory not available.\n");
            return 1;
        }

        strncpy(target,
                previous_directory,
                sizeof(target) - 1);

        target[sizeof(target) - 1] = '\0';
    }

    /*
     * cd <directory>
     */
    else
    {
        strncpy(target,
                args[1],
                sizeof(target) - 1);

        target[sizeof(target) - 1] = '\0';
    }

    /*
     * Save current directory before changing it.
     */
    char old_directory[MAX_PATH];

    if (getcwd(old_directory,
               sizeof(old_directory)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    /*
     * Validate the requested path.
     */
    if (access(target, F_OK) != 0)
    {
        printf("cd: '%s': No such file or directory.\n",
               target);

        return 1;
    }

    /*
     * Change directory.
     */
    if (chdir(target) != 0)
    {
        printf("cd: cannot change to '%s': %s\n",
               target,
               strerror(errno));

        return 1;
    }

    /*
     * Save old directory as previous directory.
     */
    strncpy(previous_directory,
            old_directory,
            sizeof(previous_directory) - 1);

    previous_directory[
        sizeof(previous_directory) - 1
    ] = '\0';

    /*
     * Update current directory.
     */
    if (!update_current_directory())
    {
        return 1;
    }

    printf("Directory changed successfully.\n");
    printf("Current : %s\n",
           current_directory);
    printf("Previous: %s\n",
           previous_directory);

    return 0;
}

/* =========================================
   ECHO BUILTIN
   ========================================= */

int builtin_echo(char *args[], int argc)
{
    for (int i = 1; i < argc; i++)
    {
        printf("%s", args[i]);

        if (i < argc - 1)
        {
            printf(" ");
        }
    }

    printf("\n");

    return 0;
}

/* =========================================
   HOME BUILTIN
   ========================================= */

int builtin_home(char *args[], int argc)
{
    (void)args;
    (void)argc;

    const char *home = getenv("HOME");

    if (home == NULL)
    {
        printf("HOME variable is not set.\n");
        return 1;
    }

    printf("HOME: %s\n", home);

    return 0;
}

/* =========================================
   STATE BUILTIN
   ========================================= */

int builtin_state(char *args[], int argc)
{
    (void)args;
    (void)argc;

    update_current_directory();

    printf("\n========== SHELL STATE ==========\n");
    printf("Current directory : %s\n",
           current_directory);

    if (strlen(previous_directory) == 0)
    {
        printf("Previous directory: Not available\n");
    }
    else
    {
        printf("Previous directory: %s\n",
               previous_directory);
    }

    printf("=================================\n");

    return 0;
}

/* =========================================
   HELP BUILTIN
   ========================================= */

int builtin_help(char *args[], int argc)
{
    (void)args;
    (void)argc;

    printf("\n========== BUILT-IN COMMANDS ==========\n");
    printf("cd [directory]  - Change directory\n");
    printf("cd -            - Return to previous directory\n");
    printf("pwd             - Display current directory\n");
    printf("echo TEXT       - Display text\n");
    printf("home            - Display HOME directory\n");
    printf("state           - Display shell directory state\n");
    printf("help            - Display help\n");
    printf("exit            - Exit Skill_11\n");
    printf("=======================================\n");

    return 0;
}

/* =========================================
   EXIT BUILTIN
   ========================================= */

int builtin_exit(char *args[], int argc)
{
    (void)args;
    (void)argc;

    return 2;
}

/* =========================================
   BUILTIN FUNCTION POINTER
   ========================================= */

typedef int (*BuiltinFunction)(char *args[],
                               int argc);

/* =========================================
   DISPATCH TABLE
   ========================================= */

typedef struct
{
    const char *name;
    BuiltinFunction function;
} BuiltinEntry;

BuiltinEntry builtin_table[] =
{
    {"cd",    builtin_cd},
    {"pwd",   builtin_pwd},
    {"echo",  builtin_echo},
    {"home",  builtin_home},
    {"state", builtin_state},
    {"help",  builtin_help},
    {"exit",  builtin_exit}
};

#define BUILTIN_COUNT \
    (sizeof(builtin_table) / sizeof(builtin_table[0]))

/* =========================================
   FIND BUILTIN
   ========================================= */

BuiltinFunction find_builtin(const char *command)
{
    for (size_t i = 0;
         i < BUILTIN_COUNT;
         i++)
    {
        if (strcmp(command,
                   builtin_table[i].name) == 0)
        {
            return builtin_table[i].function;
        }
    }

    return NULL;
}

/* =========================================
   DISPATCH BUILTIN
   ========================================= */

int dispatch_builtin(char *args[],
                     int argc)
{
    if (argc == 0)
    {
        return 0;
    }

    BuiltinFunction function =
        find_builtin(args[0]);

    if (function == NULL)
    {
        return 0;
    }

    printf("\n[Built-in detected: %s]\n",
           args[0]);

    printf("[Executing in current process]\n");

    return function(args, argc);
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[MAX_INPUT];

    /*
     * Initialize directory state.
     */
    if (!update_current_directory())
    {
        return EXIT_FAILURE;
    }

    previous_directory[0] = '\0';

    printf("========================================\n");
    printf("      SKILL_11 DIRECTORY NAVIGATION\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Change directories\n");
    printf("- Validate directory paths\n");
    printf("- Maintain current directory\n");
    printf("- Maintain previous directory\n");
    printf("- Support cd -\n");
    printf("- Built-in dispatch table\n");
    printf("- In-process execution\n");
    printf("- Shell state maintenance\n");
    printf("- Error handling\n");

    printf("\nType 'help' for commands.\n");

    while (1)
    {
        /*
         * Display current directory in prompt.
         */
        update_current_directory();

        printf("\n[%s] skill11> ",
               current_directory);

        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /*
         * Empty command.
         */
        if (strlen(input) == 0)
        {
            printf("Empty command.\n");
            continue;
        }

        /*
         * Parse command.
         */
        char *args[MAX_ARGS];

        int argc =
            parse_command(input, args);

        if (argc == 0)
        {
            free_arguments(args, argc);
            continue;
        }

        /*
         * Dispatch built-in command.
         */
        int result =
            dispatch_builtin(args, argc);

        /*
         * Check whether command was
         * actually a built-in.
         */
        BuiltinFunction check =
            find_builtin(args[0]);

        if (check == NULL)
        {
            printf("\nInvalid command: %s\n",
                   args[0]);

            printf("Type 'help' to see valid built-ins.\n");
        }

        /*
         * Exit requested.
         */
        if (result == 2)
        {
            free_arguments(args, argc);

            printf("\nExiting Skill_11...\n");

            break;
        }

        free_arguments(args, argc);
    }

    return 0;
}
