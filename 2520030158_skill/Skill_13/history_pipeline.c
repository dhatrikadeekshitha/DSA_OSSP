#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define INPUT_SIZE 512
#define MAX_HISTORY 10
#define MAX_PIPELINE 10
#define MAX_ARGS 32

/* =========================================
   HISTORY STRUCTURE
   ========================================= */

typedef struct
{
    char *commands[MAX_HISTORY];
    int count;
    int capacity;
} History;

/* =========================================
   PIPELINE COMMAND STRUCTURE
   ========================================= */

typedef struct
{
    char *argv[MAX_ARGS];
    int argc;
} PipelineCommand;

/* =========================================
   PIPELINE STRUCTURE
   ========================================= */

typedef struct
{
    PipelineCommand commands[MAX_PIPELINE];
    int count;
} Pipeline;

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
   INITIALIZE HISTORY
   ========================================= */

void init_history(History *history)
{
    history->count = 0;
    history->capacity = MAX_HISTORY;

    for (int i = 0; i < MAX_HISTORY; i++)
    {
        history->commands[i] = NULL;
    }
}

/* =========================================
   ADD COMMAND TO HISTORY
   ========================================= */

void add_history(History *history,
                 const char *command)
{
    /*
     * Ignore empty commands.
     */
    if (command == NULL ||
        strlen(command) == 0)
    {
        return;
    }

    /*
     * If history is full, remove
     * the oldest command.
     */
    if (history->count >= history->capacity)
    {
        printf("[History capacity reached: "
               "removing oldest command]\n");

        free(history->commands[0]);

        /*
         * Shift all entries left.
         */
        for (int i = 1;
             i < history->count;
             i++)
        {
            history->commands[i - 1] =
                history->commands[i];
        }

        history->count--;
    }

    /*
     * Store new command.
     */
    history->commands[history->count] =
        duplicate_string(command);

    history->count++;
}

/* =========================================
   DISPLAY HISTORY
   ========================================= */

void display_history(const History *history)
{
    printf("\n========== COMMAND HISTORY ==========\n");

    if (history->count == 0)
    {
        printf("History is empty.\n");
    }
    else
    {
        for (int i = 0;
             i < history->count;
             i++)
        {
            printf("%d  %s\n",
                   i + 1,
                   history->commands[i]);
        }
    }

    printf("Entries: %d/%d\n",
           history->count,
           history->capacity);

    printf("=====================================\n");
}

/* =========================================
   RETRIEVE HISTORY ENTRY
   ========================================= */

const char *get_history_entry(
    const History *history,
    int number)
{
    if (number < 1 ||
        number > history->count)
    {
        return NULL;
    }

    return history->commands[number - 1];
}

/* =========================================
   VALIDATE HISTORY
   ========================================= */

int validate_history(const History *history)
{
    if (history == NULL)
    {
        printf("History validation failed: "
               "NULL history.\n");

        return 0;
    }

    if (history->count < 0 ||
        history->count > history->capacity)
    {
        printf("History validation failed: "
               "invalid count.\n");

        return 0;
    }

    if (history->capacity != MAX_HISTORY)
    {
        printf("History validation failed: "
               "invalid capacity.\n");

        return 0;
    }

    for (int i = 0;
         i < history->count;
         i++)
    {
        if (history->commands[i] == NULL)
        {
            printf("History validation failed: "
                   "NULL entry at %d.\n",
                   i + 1);

            return 0;
        }
    }

    printf("History validation successful.\n");

    return 1;
}

/* =========================================
   FREE HISTORY
   ========================================= */

void free_history(History *history)
{
    for (int i = 0;
         i < history->count;
         i++)
    {
        free(history->commands[i]);
        history->commands[i] = NULL;
    }

    history->count = 0;
}

/* =========================================
   TRIM WHITESPACE
   ========================================= */

char *trim_spaces(char *text)
{
    while (*text == ' ' ||
           *text == '\t')
    {
        text++;
    }

    if (*text == '\0')
    {
        return text;
    }

    char *end =
        text + strlen(text) - 1;

    while (end > text &&
           (*end == ' ' ||
            *end == '\t'))
    {
        *end = '\0';
        end--;
    }

    return text;
}

/* =========================================
   INITIALIZE PIPELINE
   ========================================= */

void init_pipeline(Pipeline *pipeline)
{
    pipeline->count = 0;

    for (int i = 0;
         i < MAX_PIPELINE;
         i++)
    {
        pipeline->commands[i].argc = 0;

        for (int j = 0;
             j < MAX_ARGS;
             j++)
        {
            pipeline->commands[i].argv[j] = NULL;
        }
    }
}

/* =========================================
   FREE PIPELINE
   ========================================= */

void free_pipeline(Pipeline *pipeline)
{
    for (int i = 0;
         i < pipeline->count;
         i++)
    {
        for (int j = 0;
             j < pipeline->commands[i].argc;
             j++)
        {
            free(pipeline->commands[i].argv[j]);

            pipeline->commands[i].argv[j] = NULL;
        }

        pipeline->commands[i].argc = 0;
    }

    pipeline->count = 0;
}

/* =========================================
   PARSE COMMAND ARGUMENTS
   ========================================= */

int parse_arguments(char *command,
                    PipelineCommand *result)
{
    char *saveptr = NULL;

    char *token =
        strtok_r(command,
                 " \t",
                 &saveptr);

    while (token != NULL)
    {
        if (result->argc >= MAX_ARGS - 1)
        {
            printf("Parser Error: "
                   "Too many arguments.\n");

            return 0;
        }

        result->argv[result->argc] =
            duplicate_string(token);

        result->argc++;

        token =
            strtok_r(NULL,
                     " \t",
                     &saveptr);
    }

    result->argv[result->argc] = NULL;

    if (result->argc == 0)
    {
        return 0;
    }

    return 1;
}

/* =========================================
   PARSE PIPELINE
   ========================================= */

int parse_pipeline(const char *input,
                   Pipeline *pipeline)
{
    char *copy =
        duplicate_string(input);

    char *saveptr = NULL;

    char *segment =
        strtok_r(copy,
                 "|",
                 &saveptr);

    while (segment != NULL)
    {
        if (pipeline->count >= MAX_PIPELINE)
        {
            printf("Pipeline Error: "
                   "Maximum of %d commands allowed.\n",
                   MAX_PIPELINE);

            free(copy);

            return 0;
        }

        char *trimmed =
            trim_spaces(segment);

        /*
         * Detect empty pipeline stage.
         *
         * Example:
         * ls || wc
         */
        if (*trimmed == '\0')
        {
            printf("Pipeline Error: "
                   "Empty command in pipeline.\n");

            free(copy);

            return 0;
        }

        if (!parse_arguments(
                trimmed,
                &pipeline->commands[
                    pipeline->count]))
        {
            printf("Pipeline Error: "
                   "Invalid command.\n");

            free(copy);

            return 0;
        }

        pipeline->count++;

        segment =
            strtok_r(NULL,
                     "|",
                     &saveptr);
    }

    free(copy);

    if (pipeline->count == 0)
    {
        printf("Pipeline Error: "
               "No commands found.\n");

        return 0;
    }

    return 1;
}

/* =========================================
   VALIDATE PIPELINE
   ========================================= */

int validate_pipeline(
    const Pipeline *pipeline)
{
    if (pipeline == NULL)
    {
        printf("Pipeline validation failed.\n");
        return 0;
    }

    if (pipeline->count < 1 ||
        pipeline->count > MAX_PIPELINE)
    {
        printf("Pipeline validation failed: "
               "invalid command count.\n");

        return 0;
    }

    for (int i = 0;
         i < pipeline->count;
         i++)
    {
        if (pipeline->commands[i].argc == 0)
        {
            printf("Pipeline validation failed: "
                   "empty command at stage %d.\n",
                   i + 1);

            return 0;
        }

        if (pipeline->commands[i].argv[0] == NULL)
        {
            printf("Pipeline validation failed: "
                   "missing command at stage %d.\n",
                   i + 1);

            return 0;
        }
    }

    printf("Pipeline validation successful.\n");

    return 1;
}

/* =========================================
   DISPLAY PIPELINE LAYOUT
   ========================================= */

void display_pipeline(
    const Pipeline *pipeline)
{
    printf("\n========== PIPELINE LAYOUT ==========\n");

    for (int i = 0;
         i < pipeline->count;
         i++)
    {
        printf("Stage %d: ",
               i + 1);

        for (int j = 0;
             j < pipeline->commands[i].argc;
             j++)
        {
            printf("\"%s\"",
                   pipeline->commands[i].argv[j]);

            if (j <
                pipeline->commands[i].argc - 1)
            {
                printf(" ");
            }
        }

        printf("\n");
    }

    printf("\nExecution order:\n");

    for (int i = 0;
         i < pipeline->count;
         i++)
    {
        printf("Stage %d",
               i + 1);

        if (i < pipeline->count - 1)
        {
            printf(" -> ");
        }
    }

    printf("\n");

    printf("Connections: %d pipe(s)\n",
           pipeline->count - 1);

    printf("=====================================\n");
}

/* =========================================
   EXECUTE PIPELINE
   ========================================= */

int execute_pipeline(
    const Pipeline *pipeline)
{
    int command_count =
        pipeline->count;

    /*
     * One pipe is needed between each
     * pair of commands.
     */
    int pipes[MAX_PIPELINE - 1][2];

    pid_t pids[MAX_PIPELINE];

    /*
     * Create pipes.
     */
    for (int i = 0;
         i < command_count - 1;
         i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");

            /*
             * Close already-created pipes.
             */
            for (int j = 0;
                 j < i;
                 j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return 0;
        }
    }

    /*
     * Create one child for each command.
     */
    for (int i = 0;
         i < command_count;
         i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");

            for (int j = 0;
                 j < command_count - 1;
                 j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return 0;
        }

        /*
         * CHILD
         */
        if (pids[i] == 0)
        {
            /*
             * If this is not the first command,
             * read from the previous pipe.
             */
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0],
                         STDIN_FILENO) == -1)
                {
                    perror("dup2 stdin");
                    _exit(1);
                }
            }

            /*
             * If this is not the last command,
             * write to the next pipe.
             */
            if (i < command_count - 1)
            {
                if (dup2(pipes[i][1],
                         STDOUT_FILENO) == -1)
                {
                    perror("dup2 stdout");
                    _exit(1);
                }
            }

            /*
             * Close all pipe descriptors
             * after dup2().
             */
            for (int j = 0;
                 j < command_count - 1;
                 j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /*
             * Execute command.
             */
            execvp(
                pipeline->commands[i].argv[0],
                pipeline->commands[i].argv);

            /*
             * execvp() failed.
             */
            fprintf(stderr,
                    "Execution error: %s: %s\n",
                    pipeline->commands[i].argv[0],
                    strerror(errno));

            _exit(127);
        }
    }

    /*
     * PARENT
     *
     * Parent closes all pipe descriptors.
     */
    for (int i = 0;
         i < command_count - 1;
         i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /*
     * Wait for all children.
     */
    int success = 1;

    for (int i = 0;
         i < command_count;
         i++)
    {
        int status;

        if (waitpid(pids[i],
                    &status,
                    0) == -1)
        {
            perror("waitpid");
            success = 0;
            continue;
        }

        if (WIFEXITED(status))
        {
            int exit_status =
                WEXITSTATUS(status);

            if (exit_status != 0)
            {
                success = 0;
            }
        }
        else
        {
            success = 0;
        }
    }

    return success;
}

/* =========================================
   DISPLAY HISTORY ENTRY
   ========================================= */

void retrieve_history(
    const History *history,
    int number)
{
    const char *entry =
        get_history_entry(history,
                          number);

    if (entry == NULL)
    {
        printf("History entry %d does not exist.\n",
               number);

        return;
    }

    printf("\nHistory entry %d:\n",
           number);

    printf("%s\n",
           entry);
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[INPUT_SIZE];

    History history;

    init_history(&history);

    printf("========================================\n");
    printf("   SKILL_13 HISTORY & PIPELINE SYSTEM\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Command history buffer\n");
    printf("- History capacity management\n");
    printf("- History retrieval\n");
    printf("- History consistency validation\n");
    printf("- Pipeline structures\n");
    printf("- Multiple command storage\n");
    printf("- Pipeline execution order\n");
    printf("- Process connection using pipe()\n");
    printf("- Child processes using fork()\n");
    printf("- Command execution using execvp()\n");

    printf("\nBuilt-in commands:\n");
    printf("history       - Display command history\n");
    printf("history N     - Retrieve history entry N\n");
    printf("validate      - Validate history\n");
    printf("pipeline X    - Display pipeline layout\n");
    printf("exit          - Exit Skill_13\n");

    printf("\nExamples:\n");
    printf("ls | wc -l\n");
    printf("echo hello | tr a-z A-Z\n");
    printf("ls | grep Skill | wc -l\n");

    while (1)
    {
        printf("\nskill13> ");
        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /*
         * Remove empty input.
         */
        if (strlen(input) == 0)
        {
            printf("Empty command.\n");
            continue;
        }

        /*
         * Exit command is not stored in history.
         */
        if (strcmp(input, "exit") == 0)
        {
            printf("\nSaving history...\n");

            printf("History entries saved: %d\n",
                   history.count);

            printf("Cleaning up history resources...\n");

            free_history(&history);

            printf("Cleanup complete.\n");
            printf("Exiting Skill_13...\n");

            break;
        }

        /*
         * Built-in: history
         */
        if (strcmp(input, "history") == 0)
        {
            display_history(&history);
            continue;
        }

        /*
         * Built-in: validate
         */
        if (strcmp(input, "validate") == 0)
        {
            validate_history(&history);
            continue;
        }

        /*
         * Built-in: history N
         */
        if (strncmp(input,
                    "history ",
                    8) == 0)
        {
            int number;

            if (sscanf(input + 8,
                       "%d",
                       &number) == 1)
            {
                retrieve_history(&history,
                                 number);
            }
            else
            {
                printf("Usage: history N\n");
            }

            continue;
        }

        /*
         * Built-in: pipeline
         *
         * Only displays layout.
         * It does not execute.
         */
        if (strncmp(input,
                    "pipeline ",
                    9) == 0)
        {
            Pipeline pipeline;

            init_pipeline(&pipeline);

            if (parse_pipeline(input + 9,
                               &pipeline))
            {
                display_pipeline(&pipeline);
                validate_pipeline(&pipeline);
            }

            free_pipeline(&pipeline);

            continue;
        }

        /*
         * Store normal command in history.
         */
        add_history(&history,
                    input);

        /*
         * Create pipeline.
         */
        Pipeline pipeline;

        init_pipeline(&pipeline);

        /*
         * Parse pipeline.
         */
        if (!parse_pipeline(input,
                            &pipeline))
        {
            free_pipeline(&pipeline);
            continue;
        }

        /*
         * Validate pipeline.
         */
        if (!validate_pipeline(&pipeline))
        {
            free_pipeline(&pipeline);
            continue;
        }

        /*
         * Display pipeline layout.
         */
        display_pipeline(&pipeline);

        /*
         * Execute pipeline.
         */
        printf("\nExecuting pipeline...\n");

        if (execute_pipeline(&pipeline))
        {
            printf("Pipeline completed successfully.\n");
        }
        else
        {
            printf("Pipeline completed with errors.\n");
        }

        /*
         * Free pipeline resources.
         */
        free_pipeline(&pipeline);
    }

    return 0;
}
