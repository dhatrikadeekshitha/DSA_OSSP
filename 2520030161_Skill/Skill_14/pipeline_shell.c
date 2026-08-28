#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_SIZE 4096
#define MAX_PIPELINE 32
#define MAX_ARGS 64

typedef struct
{
    char *argv[MAX_ARGS];
    int argc;

    char *input_file;

    char *output_file;
    int append_output;

    char *error_file;
    int append_error;

} Command;

typedef struct
{
    Command commands[MAX_PIPELINE];
    int count;

} Pipeline;


/* =========================================
   SAFE STRING DUPLICATION
   ========================================= */

char *duplicate_string(const char *source)
{
    char *copy = strdup(source);

    if (copy == NULL)
    {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    return copy;
}


/* =========================================
   INITIALIZE COMMAND
   ========================================= */

void init_command(Command *command)
{
    memset(command, 0, sizeof(Command));
}


/* =========================================
   INITIALIZE PIPELINE
   ========================================= */

void init_pipeline(Pipeline *pipeline)
{
    memset(pipeline, 0, sizeof(Pipeline));

    for (int i = 0; i < MAX_PIPELINE; i++)
    {
        init_command(&pipeline->commands[i]);
    }
}


/* =========================================
   FREE COMMAND
   ========================================= */

void free_command(Command *command)
{
    for (int i = 0; i < command->argc; i++)
    {
        free(command->argv[i]);
    }

    free(command->input_file);
    free(command->output_file);
    free(command->error_file);

    init_command(command);
}


/* =========================================
   FREE PIPELINE
   ========================================= */

void free_pipeline(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->count; i++)
    {
        free_command(&pipeline->commands[i]);
    }

    pipeline->count = 0;
}


/* =========================================
   TOKENIZER
   ========================================= */

int next_token(const char **cursor, char **result)
{
    const char *text = *cursor;

    while (isspace((unsigned char)*text))
    {
        text++;
    }

    if (*text == '\0')
    {
        *cursor = text;
        return 0;
    }

    char buffer[INPUT_SIZE];

    size_t length = 0;

    int quote = 0;


    /* PIPE */

    if (*text == '|')
    {
        *result = duplicate_string("|");

        *cursor = text + 1;

        return 1;
    }


    /* INPUT REDIRECTION */

    if (*text == '<')
    {
        *result = duplicate_string("<");

        *cursor = text + 1;

        return 1;
    }


    /* OUTPUT REDIRECTION */

    if (*text == '>')
    {
        if (text[1] == '>')
        {
            *result = duplicate_string(">>");

            *cursor = text + 2;
        }
        else
        {
            *result = duplicate_string(">");

            *cursor = text + 1;
        }

        return 1;
    }


    /* ERROR REDIRECTION */

    if (text[0] == '2' &&
        text[1] == '>')
    {
        if (text[2] == '>')
        {
            *result = duplicate_string("2>>");

            *cursor = text + 3;
        }
        else
        {
            *result = duplicate_string("2>");

            *cursor = text + 2;
        }

        return 1;
    }


    /* NORMAL TOKEN */

    while (*text)
    {
        if (!quote &&
            isspace((unsigned char)*text))
        {
            break;
        }

        if (!quote &&
            (*text == '|' ||
             *text == '<' ||
             *text == '>'))
        {
            break;
        }

        if (!quote &&
            *text == '2' &&
            text[1] == '>')
        {
            break;
        }


        /* QUOTES */

        if (*text == '\'' ||
            *text == '"')
        {
            if (!quote)
            {
                quote = *text;

                text++;

                continue;
            }

            if (quote == *text)
            {
                quote = 0;

                text++;

                continue;
            }
        }


        /* ESCAPE */

        if (*text == '\\' &&
            text[1])
        {
            text++;

            buffer[length++] = *text;

            text++;
        }
        else
        {
            buffer[length++] = *text;

            text++;
        }


        if (length >=
            sizeof(buffer) - 1)
        {
            fprintf(stderr,
                    "Parser error: token too long.\n");

            return -1;
        }
    }


    if (quote)
    {
        fprintf(stderr,
                "Parser error: unmatched quote.\n");

        return -1;
    }


    buffer[length] = '\0';

    *result = duplicate_string(buffer);

    *cursor = text;

    return 1;
}


/* =========================================
   SET REDIRECTION
   ========================================= */

int set_redirection(
    Command *command,
    const char *operator,
    const char *file)
{
    if (file == NULL ||
        *file == '\0')
    {
        fprintf(stderr,
                "Parser error: missing file after %s.\n",
                operator);

        return 0;
    }


    /* INPUT */

    if (strcmp(operator, "<") == 0)
    {
        if (command->input_file != NULL)
        {
            fprintf(stderr,
                    "Parser error: duplicate input redirection.\n");

            return 0;
        }

        command->input_file =
            duplicate_string(file);
    }


    /* OUTPUT */

    else if (strcmp(operator, ">") == 0 ||
             strcmp(operator, ">>") == 0)
    {
        if (command->output_file != NULL)
        {
            fprintf(stderr,
                    "Parser error: duplicate output redirection.\n");

            return 0;
        }

        command->output_file =
            duplicate_string(file);

        command->append_output =
            strcmp(operator, ">>") == 0;
    }


    /* ERROR */

    else if (strcmp(operator, "2>") == 0 ||
             strcmp(operator, "2>>") == 0)
    {
        if (command->error_file != NULL)
        {
            fprintf(stderr,
                    "Parser error: duplicate error redirection.\n");

            return 0;
        }

        command->error_file =
            duplicate_string(file);

        command->append_error =
            strcmp(operator, "2>>") == 0;
    }

    else
    {
        return 0;
    }

    return 1;
}


/* =========================================
   PARSE PIPELINE
   ========================================= */

int parse_pipeline(
    const char *input,
    Pipeline *pipeline)
{
    const char *cursor = input;

    char *token = NULL;

    int stage = 0;

    pipeline->count = 1;


    while (1)
    {
        int result =
            next_token(&cursor, &token);

        if (result < 0)
        {
            return 0;
        }

        if (result == 0)
        {
            break;
        }


        /* PIPE */

        if (strcmp(token, "|") == 0)
        {
            free(token);

            if (pipeline
                    ->commands[stage]
                    .argc == 0)
            {
                fprintf(stderr,
                        "Pipeline error: empty command before '|'.\n");

                return 0;
            }


            stage++;


            if (stage >= MAX_PIPELINE)
            {
                fprintf(stderr,
                        "Pipeline error: maximum %d stages.\n",
                        MAX_PIPELINE);

                return 0;
            }


            pipeline->count =
                stage + 1;

            continue;
        }


        /* REDIRECTION */

        if (strcmp(token, "<") == 0 ||
            strcmp(token, ">") == 0 ||
            strcmp(token, ">>") == 0 ||
            strcmp(token, "2>") == 0 ||
            strcmp(token, "2>>") == 0)
        {
            char *file = NULL;

            int result2 =
                next_token(&cursor, &file);


            if (result2 != 1)
            {
                fprintf(stderr,
                        "Parser error: missing redirection target.\n");

                free(token);
                free(file);

                return 0;
            }


            if (strcmp(file, "|") == 0 ||
                strcmp(file, "<") == 0 ||
                strcmp(file, ">") == 0 ||
                strcmp(file, ">>") == 0)
            {
                fprintf(stderr,
                        "Parser error: invalid redirection target.\n");

                free(token);
                free(file);

                return 0;
            }


            int valid =
                set_redirection(
                    &pipeline->commands[stage],
                    token,
                    file);


            free(token);
            free(file);


            if (!valid)
            {
                return 0;
            }

            continue;
        }


        /* COMMAND ARGUMENT */

        Command *command =
            &pipeline->commands[stage];


        if (command->argc >=
            MAX_ARGS - 1)
        {
            fprintf(stderr,
                    "Parser error: too many arguments.\n");

            free(token);

            return 0;
        }


        command->argv[
            command->argc++] = token;

        command->argv[
            command->argc] = NULL;
    }


    if (pipeline
            ->commands[stage]
            .argc == 0)
    {
        fprintf(stderr,
                "Pipeline error: empty final command.\n");

        return 0;
    }


    return 1;
}


/* =========================================
   REDIRECT STREAMS
   ========================================= */

int redirect_streams(
    const Command *command)
{
    int fd;


    /* INPUT */

    if (command->input_file != NULL)
    {
        fd = open(
            command->input_file,
            O_RDONLY);

        if (fd < 0)
        {
            perror(command->input_file);

            return 0;
        }


        if (dup2(fd,
                 STDIN_FILENO) < 0)
        {
            perror("dup2 stdin");

            close(fd);

            return 0;
        }

        close(fd);
    }


    /* OUTPUT */

    if (command->output_file != NULL)
    {
        int flags =
            O_WRONLY |
            O_CREAT |
            (command->append_output
                 ? O_APPEND
                 : O_TRUNC);


        fd = open(
            command->output_file,
            flags,
            0666);


        if (fd < 0)
        {
            perror(command->output_file);

            return 0;
        }


        if (dup2(fd,
                 STDOUT_FILENO) < 0)
        {
            perror("dup2 stdout");

            close(fd);

            return 0;
        }

        close(fd);
    }


    /* ERROR */

    if (command->error_file != NULL)
    {
        int flags =
            O_WRONLY |
            O_CREAT |
            (command->append_error
                 ? O_APPEND
                 : O_TRUNC);


        fd = open(
            command->error_file,
            flags,
            0666);


        if (fd < 0)
        {
            perror(command->error_file);

            return 0;
        }


        if (dup2(fd,
                 STDERR_FILENO) < 0)
        {
            perror("dup2 stderr");

            close(fd);

            return 0;
        }

        close(fd);
    }


    return 1;
}


/* =========================================
   EXECUTE MULTIPLE PIPELINE
   ========================================= */

int execute_pipeline(
    const Pipeline *pipeline)
{
    int command_count =
        pipeline->count;


    /*
     * N commands require
     * N-1 pipes.
     */

    int pipes[MAX_PIPELINE - 1][2];

    pid_t pids[MAX_PIPELINE];


    /* =====================================
       CREATE ALL PIPES
       ===================================== */

    for (int i = 0;
         i < command_count - 1;
         i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");


            /*
             * Cleanup already-created
             * descriptors.
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


    /* =====================================
       CREATE CHILD PROCESSES
       ===================================== */

    for (int i = 0;
         i < command_count;
         i++)
    {
        pids[i] = fork();


        /* FORK ERROR */

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


            for (int j = 0;
                 j < i;
                 j++)
            {
                waitpid(
                    pids[j],
                    NULL,
                    0);
            }


            return 0;
        }


        /* =================================
           CHILD PROCESS
           ================================= */

        if (pids[i] == 0)
        {
            /*
             * Read from previous pipe.
             */

            if (i > 0)
            {
                if (dup2(
                        pipes[i - 1][0],
                        STDIN_FILENO) < 0)
                {
                    perror(
                        "dup2 stdin pipe");

                    _exit(126);
                }
            }


            /*
             * Write to next pipe.
             */

            if (i < command_count - 1)
            {
                if (dup2(
                        pipes[i][1],
                        STDOUT_FILENO) < 0)
                {
                    perror(
                        "dup2 stdout pipe");

                    _exit(126);
                }
            }


            /*
             * Explicit redirection
             * overrides pipe stream.
             */

            if (!redirect_streams(
                    &pipeline->commands[i]))
            {
                _exit(126);
            }


            /*
             * Close ALL pipe descriptors
             * inside child.
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
                pipeline->commands[i]
                    .argv[0],
                pipeline->commands[i]
                    .argv);


            /*
             * execvp failed.
             */

            fprintf(
                stderr,
                "%s: %s\n",
                pipeline->commands[i]
                    .argv[0],
                strerror(errno));


            _exit(
                errno == ENOENT
                    ? 127
                    : 126);
        }
    }


    /* =====================================
       PARENT CLOSES ALL PIPE DESCRIPTORS
       ===================================== */

    for (int i = 0;
         i < command_count - 1;
         i++)
    {
        close(pipes[i][0]);

        close(pipes[i][1]);
    }


    /*
     * Parent waits for every child.
     */

    int success = 1;


    for (int i = 0;
         i < command_count;
         i++)
    {
        int status;


        if (waitpid(
                pids[i],
                &status,
                0) < 0)
        {
            perror("waitpid");

            success = 0;

            continue;
        }


        if (!WIFEXITED(status) ||
            WEXITSTATUS(status) != 0)
        {
            success = 0;
        }
    }


    return success;
}


/* =========================================
   DISPLAY PIPELINE
   ========================================= */

void display_pipeline(
    const Pipeline *pipeline)
{
    printf("\n========== PIPELINE ==========\n");


    for (int i = 0;
         i < pipeline->count;
         i++)
    {
        printf("Stage %d: ",
               i + 1);


        for (int j = 0;
             j < pipeline
                     ->commands[i]
                     .argc;
             j++)
        {
            printf("%s ",
                   pipeline
                       ->commands[i]
                       .argv[j]);
        }


        printf("\n");
    }


    printf(
        "Stages: %d\n",
        pipeline->count);


    printf(
        "Pipes: %d\n",
        pipeline->count - 1);


    printf(
        "==============================\n");
}


/* =========================================
   MAIN
   ========================================= */

int main(void)
{
    char input[INPUT_SIZE];


    printf(
        "===============================================\n");

    printf(
        " SKILL_14 - MULTI-PIPE & STREAM REDIRECTION\n");

    printf(
        "===============================================\n");


    printf(
        "Supports: |, <, >, >>, 2>, 2>\n");

    printf(
        "Maximum pipeline stages: %d\n",
        MAX_PIPELINE);


    printf(
        "\nExamples:\n");

    printf(
        "ls | grep Skill | wc -l\n");

    printf(
        "cat input.txt | tr a-z A-Z | sort > output.txt\n");

    printf(
        "seq 1 100 | cat | cat | wc -l\n");


    printf(
        "\nType exit to quit.\n");


    while (1)
    {
        printf("\nskill14> ");

        fflush(stdout);


        if (fgets(
                input,
                sizeof(input),
                stdin) == NULL)
        {
            break;
        }


        input[
            strcspn(input, "\n")
        ] = '\0';


        if (strlen(input) == 0)
        {
            continue;
        }


        if (strcmp(input, "exit") == 0)
        {
            break;
        }


        Pipeline pipeline;

        init_pipeline(&pipeline);


        /*
         * Parse pipeline.
         */

        if (!parse_pipeline(
                input,
                &pipeline))
        {
            free_pipeline(&pipeline);

            continue;
        }


        /*
         * Display structure.
         */

        display_pipeline(
            &pipeline);


        printf(
            "Executing pipeline...\n");


        /*
         * Execute.
         */

        if (execute_pipeline(
                &pipeline))
        {
            printf(
                "Pipeline completed successfully.\n");
        }
        else
        {
            printf(
                "Pipeline completed with errors.\n");
        }


        /*
         * Cleanup.
         */

        free_pipeline(
            &pipeline);
    }


    printf(
        "\nSkill_14 exited.\n");

    printf(
        "All pipeline resources cleaned up.\n");


    return 0;
}
