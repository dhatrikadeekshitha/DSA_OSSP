#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#define INPUT_SIZE 512
#define MAX_ARGS 64

/* =========================================
   SHELL STATE
   ========================================= */

typedef struct
{
    char current_directory[PATH_MAX];
    char previous_directory[PATH_MAX];
} ShellState;

ShellState shell_state;

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
   UPDATE CURRENT DIRECTORY
   ========================================= */

int update_current_directory()
{
    if (getcwd(shell_state.current_directory,
               sizeof(shell_state.current_directory)) == NULL)
    {
        perror("getcwd");
        return 0;
    }

    return 1;
}

/* =========================================
   DISPLAY CURRENT DIRECTORY
   ========================================= */

void display_current_directory()
{
    if (!update_current_directory())
    {
        return;
    }

    printf("\n========== CURRENT DIRECTORY ==========\n");
    printf("%s\n", shell_state.current_directory);
    printf("=======================================\n");
}

/* =========================================
   PARSE COMMAND
   ========================================= */

int parse_command(const char *input,
                  char *args[])
{
    char *copy = duplicate_string(input);

    int argc = 0;

    char *token = strtok(copy, " \t");

    while (token != NULL &&
           argc < MAX_ARGS - 1)
    {
        args[argc] =
            duplicate_string(token);

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

void free_arguments(char *args[],
                    int argc)
{
    for (int i = 0; i < argc; i++)
    {
        free(args[i]);
        args[i] = NULL;
    }
}

/* =========================================
   VALIDATE VARIABLE NAME
   ========================================= */

int valid_variable_name(const char *name)
{
    if (name == NULL ||
        name[0] == '\0')
    {
        return 0;
    }

    /*
     * First character must be a letter
     * or underscore.
     */
    if (!isalpha((unsigned char)name[0]) &&
        name[0] != '_')
    {
        return 0;
    }

    /*
     * Remaining characters can be:
     * letters, digits, underscore.
     */
    for (int i = 1;
         name[i] != '\0';
         i++)
    {
        if (!isalnum((unsigned char)name[i]) &&
            name[i] != '_')
        {
            return 0;
        }
    }

    return 1;
}

/* =========================================
   DISPLAY ENVIRONMENT VARIABLE
   ========================================= */

void display_variable(const char *name)
{
    const char *value =
        getenv(name);

    if (value == NULL)
    {
        printf("%s is not set.\n",
               name);
    }
    else
    {
        printf("%s=%s\n",
               name,
               value);
    }
}

/* =========================================
   EXPORT VARIABLE
   ========================================= */

int builtin_export(char *args[],
                   int argc)
{
    if (argc < 2)
    {
        printf("Usage: export NAME=VALUE\n");
        return 1;
    }

    /*
     * Find '=' in NAME=VALUE.
     */
    char *equals =
        strchr(args[1], '=');

    if (equals == NULL)
    {
        /*
         * If only a variable name is given,
         * display its current value.
         */
        if (!valid_variable_name(args[1]))
        {
            printf("export: invalid variable name: %s\n",
                   args[1]);

            return 1;
        }

        display_variable(args[1]);

        return 0;
    }

    /*
     * Determine variable name length.
     */
    int name_length =
        equals - args[1];

    if (name_length <= 0 ||
        name_length >= 128)
    {
        printf("export: invalid variable name.\n");
        return 1;
    }

    char name[128];

    strncpy(name,
            args[1],
            name_length);

    name[name_length] = '\0';

    /*
     * Validate variable name.
     */
    if (!valid_variable_name(name))
    {
        printf("export: invalid variable name: %s\n",
               name);

        return 1;
    }

    /*
     * Value begins after '='.
     */
    const char *value =
        equals + 1;

    /*
     * setenv() updates the process environment.
     *
     * overwrite = 1 means an existing variable
     * will be updated.
     */
    if (setenv(name,
               value,
               1) != 0)
    {
        printf("export: unable to set variable: %s\n",
               strerror(errno));

        return 1;
    }

    printf("Exported successfully:\n");
    printf("%s=%s\n",
           name,
           value);

    return 0;
}

/* =========================================
   SHOW ALL SELECTED ENVIRONMENT VARIABLES
   ========================================= */
int display_environment(char *args[], int argc)
{
    (void)args;
    (void)argc;

    printf("\n========== ENVIRONMENT ==========\n");

    display_variable("HOME");
    display_variable("USER");
    display_variable("PATH");
    display_variable("SHELL");

    printf("=================================\n");

    return 0;
}

/* =========================================
   CD BUILTIN
   ========================================= */

int builtin_cd(char *args[],
               int argc)
{
    char target[PATH_MAX];

    /*
     * Save current directory.
     */
    char old_directory[PATH_MAX];

    if (getcwd(old_directory,
               sizeof(old_directory)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    /*
     * cd with no argument.
     */
    if (argc < 2)
    {
        const char *home =
            getenv("HOME");

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
     * cd -
     */
    else if (strcmp(args[1], "-") == 0)
    {
        if (strlen(shell_state.previous_directory) == 0)
        {
            printf("cd: previous directory not available.\n");
            return 1;
        }

        strncpy(target,
                shell_state.previous_directory,
                sizeof(target) - 1);

        target[sizeof(target) - 1] = '\0';
    }

    /*
     * cd directory
     */
    else
    {
        strncpy(target,
                args[1],
                sizeof(target) - 1);

        target[sizeof(target) - 1] = '\0';
    }

    /*
     * Validate path.
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
        printf("cd: cannot change directory: %s\n",
               strerror(errno));

        return 1;
    }

    /*
     * Save previous directory.
     */
    strncpy(shell_state.previous_directory,
            old_directory,
            sizeof(shell_state.previous_directory) - 1);

    shell_state.previous_directory[
        sizeof(shell_state.previous_directory) - 1
    ] = '\0';

    /*
     * Update current directory.
     */
    update_current_directory();

    printf("Directory changed successfully.\n");
    printf("Current : %s\n",
           shell_state.current_directory);
    printf("Previous: %s\n",
           shell_state.previous_directory);

    return 0;
}

/* =========================================
   PWD BUILTIN
   ========================================= */

int builtin_pwd(char *args[],
                int argc)
{
    (void)args;
    (void)argc;

    display_current_directory();

    return 0;
}

/* =========================================
   ECHO BUILTIN
   ========================================= */

int builtin_echo(char *args[],
                 int argc)
{
    for (int i = 1;
         i < argc;
         i++)
    {
        printf("%s",
               args[i]);

        if (i < argc - 1)
        {
            printf(" ");
        }
    }

    printf("\n");

    return 0;
}

/* =========================================
   STATE BUILTIN
   ========================================= */

int builtin_state(char *args[],
                  int argc)
{
    (void)args;
    (void)argc;

    update_current_directory();

    printf("\n========== SHELL STATE ==========\n");

    printf("Current directory : %s\n",
           shell_state.current_directory);

    if (strlen(shell_state.previous_directory) == 0)
    {
        printf("Previous directory: Not available\n");
    }
    else
    {
        printf("Previous directory: %s\n",
               shell_state.previous_directory);
    }

    printf("=================================\n");

    return 0;
}

/* =========================================
   TEST ENVIRONMENT IN CHILD PROCESS
   ========================================= */

int builtin_test_export(char *args[],
                        int argc)
{
    (void)args;
    (void)argc;

    printf("\n===== CHILD PROCESS EXPORT TEST =====\n");

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    /*
     * Child process.
     */
    if (pid == 0)
    {
        const char *value =
            getenv("SKILL12_TEST");

        if (value == NULL)
        {
            printf("Child: SKILL12_TEST is not set.\n");
        }
        else
        {
            printf("Child: SKILL12_TEST=%s\n",
                   value);
        }

        _exit(0);
    }

    /*
     * Parent waits for child.
     */
    int status;

    if (waitpid(pid,
                &status,
                0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        printf("Child exited with status: %d\n",
               WEXITSTATUS(status));
    }

    printf("====================================\n");

    return 0;
}

/* =========================================
   HELP BUILTIN
   ========================================= */

int builtin_help(char *args[],
                 int argc)
{
    (void)args;
    (void)argc;

    printf("\n========== BUILT-IN COMMANDS ==========\n");

    printf("pwd                    - Display current directory\n");
    printf("cd [directory]         - Change directory\n");
    printf("cd -                   - Return to previous directory\n");
    printf("echo TEXT              - Display text\n");
    printf("export NAME=VALUE      - Export environment variable\n");
    printf("export NAME            - Display variable value\n");
    printf("env                    - Display selected environment\n");
    printf("state                  - Display shell state\n");
    printf("testexport             - Test variable in child process\n");
    printf("help                   - Display help\n");
    printf("exit                   - Exit Skill_12\n");

    printf("=======================================\n");

    return 0;
}

/* =========================================
   EXIT BUILTIN
   ========================================= */

int builtin_exit(char *args[],
                 int argc)
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
    {"pwd",         builtin_pwd},
    {"cd",          builtin_cd},
    {"echo",        builtin_echo},
    {"export",      builtin_export},
    {"env",          display_environment},
    {"state",       builtin_state},
    {"testexport",  builtin_test_export},
    {"help",        builtin_help},
    {"exit",        builtin_exit}
};

#define BUILTIN_COUNT \
    (sizeof(builtin_table) / sizeof(builtin_table[0]))

/* =========================================
   FIND BUILTIN
   ========================================= */

BuiltinFunction find_builtin(
    const char *command)
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
   MAIN
   ========================================= */

int main()
{
    char input[INPUT_SIZE];

    /*
     * Initialize shell state.
     */
    if (!update_current_directory())
    {
        return EXIT_FAILURE;
    }

    shell_state.previous_directory[0] = '\0';

    printf("========================================\n");
    printf("       SKILL_12 EXPORT & SHELL STATE\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Retrieve current directory\n");
    printf("- Display path\n");
    printf("- Process exit requests\n");
    printf("- Save shell state\n");
    printf("- Cleanup resources\n");
    printf("- Parse export syntax\n");
    printf("- Validate variable names\n");
    printf("- Update environment variables\n");
    printf("- Handle existing variables\n");
    printf("- Export variables to child processes\n");

    printf("\nType 'help' for commands.\n");

    while (1)
    {
        update_current_directory();

        printf("\n[%s] skill12> ",
               shell_state.current_directory);

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
         * Parse input.
         */
        char *args[MAX_ARGS];

        int argc =
            parse_command(input,
                          args);

        if (argc == 0)
        {
            free_arguments(args, argc);
            continue;
        }

        /*
         * Find built-in command.
         */
        BuiltinFunction function =
            find_builtin(args[0]);

        if (function == NULL)
        {
            printf("Invalid command: %s\n",
                   args[0]);

            printf("Type 'help' to see available commands.\n");

            free_arguments(args, argc);
            continue;
        }

        printf("\n[Built-in detected: %s]\n",
               args[0]);

        printf("[Executing in current process]\n");

        int result =
            function(args, argc);

        /*
         * Free command arguments.
         */
        free_arguments(args, argc);

        /*
         * Process exit request.
         */
        if (result == 2)
        {
            printf("\nSaving shell state...\n");

            update_current_directory();

            printf("Current directory: %s\n",
                   shell_state.current_directory);

            printf("Cleaning up resources...\n");

            /*
             * No dynamically allocated shell
             * state remains at this point.
             */

            printf("Cleanup complete.\n");
            printf("Exiting Skill_12...\n");

            break;
        }
    }

    /*
     * Final cleanup.
     */
    memset(&shell_state,
           0,
           sizeof(shell_state));

    return 0;
}
