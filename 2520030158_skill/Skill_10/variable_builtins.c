#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

#define MAX_INPUT 512
#define MAX_ARGS 64
#define MAX_VARIABLES 50

/* =========================================
   USER-DEFINED VARIABLE
   ========================================= */

typedef struct Variable
{
    char *name;
    char *value;
} Variable;

Variable variables[MAX_VARIABLES];
int variable_count = 0;

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
   FREE USER VARIABLES
   ========================================= */

void free_variables()
{
    for (int i = 0; i < variable_count; i++)
    {
        free(variables[i].name);
        free(variables[i].value);
    }

    variable_count = 0;
}

/* =========================================
   FIND USER VARIABLE
   ========================================= */

int find_user_variable(const char *name)
{
    for (int i = 0; i < variable_count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

/* =========================================
   GET VARIABLE VALUE
   ========================================= */

const char *get_variable_value(const char *name)
{
    /*
     * First search user-defined variables.
     */
    int index = find_user_variable(name);

    if (index >= 0)
    {
        return variables[index].value;
    }

    /*
     * Then search environment variables.
     */
    const char *environment_value =
        getenv(name);

    if (environment_value != NULL)
    {
        return environment_value;
    }

    /*
     * Undefined variable.
     */
    return NULL;
}

/* =========================================
   SET USER VARIABLE
   ========================================= */

void set_variable(const char *name,
                  const char *value)
{
    int index = find_user_variable(name);

    /*
     * Update existing variable.
     */
    if (index >= 0)
    {
        free(variables[index].value);

        variables[index].value =
            duplicate_string(value);

        return;
    }

    /*
     * Create a new variable.
     */
    if (variable_count >= MAX_VARIABLES)
    {
        printf("Error: Maximum number of variables reached.\n");
        return;
    }

    variables[variable_count].name =
        duplicate_string(name);

    variables[variable_count].value =
        duplicate_string(value);

    variable_count++;
}

/* =========================================
   DISPLAY VARIABLES
   ========================================= */

void display_variables()
{
    printf("\n========== USER VARIABLES ==========\n");

    if (variable_count == 0)
    {
        printf("No user-defined variables.\n");
    }

    for (int i = 0; i < variable_count; i++)
    {
        printf("%s=%s\n",
               variables[i].name,
               variables[i].value);
    }

    printf("====================================\n");
}

/* =========================================
   CHECK VALID VARIABLE NAME
   ========================================= */

int valid_variable_name(const char *name)
{
    if (name == NULL ||
        name[0] == '\0')
    {
        return 0;
    }

    if (!isalpha((unsigned char)name[0]) &&
        name[0] != '_')
    {
        return 0;
    }

    for (int i = 1; name[i] != '\0'; i++)
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
   VARIABLE EXPANSION
   ========================================= */

char *expand_variables(const char *input)
{
    size_t capacity = 128;
    size_t length = 0;

    char *result = malloc(capacity);

    if (result == NULL)
    {
        perror("malloc");
        return NULL;
    }

    int i = 0;

    while (input[i] != '\0')
    {
        /*
         * Detect variable reference.
         */
        if (input[i] == '$')
        {
            char variable_name[128];
            int name_length = 0;

            i++;

            /*
             * ${VARIABLE} format.
             */
            if (input[i] == '{')
            {
                i++;

                while (input[i] != '\0' &&
                       input[i] != '}' &&
                       name_length < 127)
                {
                    variable_name[name_length++] =
                        input[i];

                    i++;
                }

                variable_name[name_length] = '\0';

                if (input[i] == '}')
                {
                    i++;
                }
                else
                {
                    printf("Expansion Error: Missing '}'.\n");
                }
            }

            /*
             * $VARIABLE format.
             */
            else
            {
                while (input[i] != '\0' &&
                       (isalnum((unsigned char)input[i]) ||
                        input[i] == '_') &&
                       name_length < 127)
                {
                    variable_name[name_length++] =
                        input[i];

                    i++;
                }

                variable_name[name_length] = '\0';
            }

            /*
             * Empty variable name.
             */
            if (name_length == 0)
            {
                if (length + 1 >= capacity)
                {
                    capacity *= 2;

                    char *temp =
                        realloc(result, capacity);

                    if (temp == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    result = temp;
                }

                result[length++] = '$';

                continue;
            }

            /*
             * Retrieve variable value.
             */
            const char *value =
                get_variable_value(variable_name);

            if (value == NULL)
            {
                /*
                 * Undefined variables expand
                 * to an empty string.
                 */
                printf("[Undefined variable: %s]\n",
                       variable_name);

                value = "";
            }

            /*
             * Add expanded value to result.
             */
            for (int j = 0;
                 value[j] != '\0';
                 j++)
            {
                if (length + 1 >= capacity)
                {
                    capacity *= 2;

                    char *temp =
                        realloc(result, capacity);

                    if (temp == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    result = temp;
                }

                result[length++] = value[j];
            }

            continue;
        }

        /*
         * Normal character.
         */
        if (length + 1 >= capacity)
        {
            capacity *= 2;

            char *temp =
                realloc(result, capacity);

            if (temp == NULL)
            {
                free(result);
                return NULL;
            }

            result = temp;
        }

        result[length++] = input[i];

        i++;
    }

    result[length] = '\0';

    return result;
}

/* =========================================
   TOKENIZE AND EXPAND
   ========================================= */

int parse_command(const char *input,
                  char *args[])
{
    char *expanded =
        expand_variables(input);

    if (expanded == NULL)
    {
        return -1;
    }

    printf("\n========== EXPANSION RESULT ==========\n");
    printf("Original : %s\n", input);
    printf("Expanded : %s\n", expanded);
    printf("======================================\n");

    int argc = 0;

    char *token =
        strtok(expanded, " \t");

    while (token != NULL &&
           argc < MAX_ARGS - 1)
    {
        args[argc] =
            duplicate_string(token);

        argc++;

        token =
            strtok(NULL, " \t");
    }

    args[argc] = NULL;

    free(expanded);

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
   BUILTIN FUNCTION TYPE
   ========================================= */

typedef int (*BuiltinFunction)(char *args[],
                               int argc);

/* =========================================
   BUILTIN FUNCTIONS
   ========================================= */

int builtin_cd(char *args[], int argc)
{
    const char *directory;

    if (argc < 2)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            printf("cd: HOME is not set.\n");
            return 1;
        }
    }
    else
    {
        directory = args[1];
    }

    if (chdir(directory) != 0)
    {
        perror("cd");
        return 1;
    }

    printf("Directory changed successfully.\n");

    return 0;
}

/* ----------------------------------------- */

int builtin_pwd(char *args[], int argc)
{
    (void)args;
    (void)argc;

    char current_directory[PATH_MAX];

    if (getcwd(current_directory,
               sizeof(current_directory)) == NULL)
    {
        perror("pwd");
        return 1;
    }

    printf("%s\n", current_directory);

    return 0;
}

/* ----------------------------------------- */

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

/* ----------------------------------------- */

int builtin_set(char *args[], int argc)
{
    if (argc < 2)
    {
        printf("Usage: set NAME=VALUE\n");
        return 1;
    }

    char *equals =
        strchr(args[1], '=');

    if (equals == NULL)
    {
        printf("Usage: set NAME=VALUE\n");
        return 1;
    }

    int name_length =
        equals - args[1];

    if (name_length <= 0 ||
        name_length >= 128)
    {
        printf("Invalid variable name.\n");
        return 1;
    }

    char name[128];

    strncpy(name,
            args[1],
            name_length);

    name[name_length] = '\0';

    const char *value =
        equals + 1;

    if (!valid_variable_name(name))
    {
        printf("Invalid variable name: %s\n",
               name);

        return 1;
    }

    set_variable(name, value);

    printf("Variable set: %s=%s\n",
           name,
           value);

    return 0;
}

/* ----------------------------------------- */

int builtin_vars(char *args[], int argc)
{
    (void)args;
    (void)argc;

    display_variables();

    return 0;
}

/* ----------------------------------------- */

int builtin_help(char *args[], int argc)
{
    (void)args;
    (void)argc;

    printf("\n========== BUILT-IN COMMANDS ==========\n");

    printf("cd [directory]  - Change directory\n");
    printf("pwd             - Show current directory\n");
    printf("echo TEXT       - Display text\n");
    printf("set X=VALUE     - Create/update variable\n");
    printf("vars            - Display user variables\n");
    printf("help            - Show this help\n");
    printf("exit            - Exit Skill_10\n");

    printf("\nVariable expansion:\n");
    printf("$USER\n");
    printf("$HOME\n");
    printf("${USER}\n");
    printf("${NAME}\n");

    printf("=======================================\n");

    return 0;
}

/* ----------------------------------------- */

int builtin_exit(char *args[], int argc)
{
    (void)args;
    (void)argc;

    return 2;
}

/* =========================================
   BUILTIN DISPATCH TABLE
   ========================================= */

typedef struct
{
    const char *name;
    BuiltinFunction function;
} BuiltinEntry;

BuiltinEntry builtin_table[] =
{
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"echo", builtin_echo},
    {"set", builtin_set},
    {"vars", builtin_vars},
    {"help", builtin_help},
    {"exit", builtin_exit}
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

    printf("========================================\n");
    printf("      SKILL_10 VARIABLE & BUILT-INS\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Variable reference detection\n");
    printf("- Environment variable expansion\n");
    printf("- User-defined variables\n");
    printf("- Undefined variable handling\n");
    printf("- ${VARIABLE} support\n");
    printf("- Token updates after expansion\n");
    printf("- Built-in command dispatch table\n");
    printf("- In-process command execution\n");
    printf("- Shell state maintenance\n");

    printf("\nType 'help' for commands.\n");

    while (1)
    {
        printf("\nskill10> ");
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
         * Parse and expand variables.
         */
        char *args[MAX_ARGS];

        int argc =
            parse_command(input, args);

        if (argc < 0)
        {
            continue;
        }

        if (argc == 0)
        {
            free_arguments(args, argc);
            continue;
        }

        /*
         * Try built-in dispatch.
         */
        int result =
            dispatch_builtin(args, argc);

        /*
         * result = 2 means exit.
         */
        if (result == 2)
        {
            free_arguments(args, argc);

            printf("Exiting Skill_10...\n");

            break;
        }

        /*
         * If command was not a built-in,
         * report it.
         */
        BuiltinFunction check =
            find_builtin(args[0]);

        if (check == NULL)
        {
            printf("\nInvalid command: %s\n",
                   args[0]);

            printf("Type 'help' to see valid built-ins.\n");
        }

        free_arguments(args, argc);
    }

    free_variables();

    return 0;
}
