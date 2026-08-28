/*
 * Skill-17
 *
 * Objective 1:
 * Parse combined redirection, duplicate file descriptors,
 * merge streams, verify ordering, handle edge cases,
 * and test integrated outputs.
 *
 * Objective 2:
 * Parse complex commands, combine pipes and redirections,
 * resolve execution order, validate syntax,
 * generate execution plans, and test scenarios.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_CMDS 16

typedef struct {
    char *args[MAX_ARGS];
    int argc;

    char *input_file;
    char *output_file;
    char *error_file;

    int append_output;
    int append_error;

    int stderr_to_stdout;
} Command;


/* Remove leading and trailing spaces */
char *trim(char *str)
{
    while (*str == ' ' || *str == '\t')
        str++;

    char *end = str + strlen(str) - 1;

    while (end >= str &&
           (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }

    return str;
}


/* Initialize command structure */
void init_command(Command *cmd)
{
    memset(cmd, 0, sizeof(Command));
}


/* Parse one command */
int parse_command(char *text, Command *cmd)
{
    init_command(cmd);

    char *token = strtok(text, " \t\n");

    while (token != NULL) {

        /* Input redirection */
        if (strcmp(token, "<") == 0) {

            token = strtok(NULL, " \t\n");

            if (token == NULL) {
                printf("Syntax Error: missing input file\n");
                return -1;
            }

            cmd->input_file = token;
        }

        /* Output redirection */
        else if (strcmp(token, ">") == 0) {

            token = strtok(NULL, " \t\n");

            if (token == NULL) {
                printf("Syntax Error: missing output file\n");
                return -1;
            }

            cmd->output_file = token;
            cmd->append_output = 0;
        }

        /* Append output */
        else if (strcmp(token, ">>") == 0) {

            token = strtok(NULL, " \t\n");

            if (token == NULL) {
                printf("Syntax Error: missing append file\n");
                return -1;
            }

            cmd->output_file = token;
            cmd->append_output = 1;
        }

        /* stderr redirection */
        else if (strcmp(token, "2>") == 0) {

            token = strtok(NULL, " \t\n");

            if (token == NULL) {
                printf("Syntax Error: missing stderr file\n");
                return -1;
            }

            cmd->error_file = token;
            cmd->append_error = 0;
        }

        /* stderr append */
        else if (strcmp(token, "2>>") == 0) {

            token = strtok(NULL, " \t\n");

            if (token == NULL) {
                printf("Syntax Error: missing stderr append file\n");
                return -1;
            }

            cmd->error_file = token;
            cmd->append_error = 1;
        }

        /* Combined stdout + stderr */
        else if (strcmp(token, "2>&1") == 0) {

            cmd->stderr_to_stdout = 1;
        }

        /* Normal argument */
        else {

            if (cmd->argc >= MAX_ARGS - 1) {
                printf("Error: too many arguments\n");
                return -1;
            }

            cmd->args[cmd->argc++] = token;
        }

        token = strtok(NULL, " \t\n");
    }

    cmd->args[cmd->argc] = NULL;

    if (cmd->argc == 0) {
        return -1;
    }

    return 0;
}


/* Display execution plan */
void show_plan(Command *cmds, int count)
{
    printf("\n========== EXECUTION PLAN ==========\n");

    for (int i = 0; i < count; i++) {

        printf("\nCommand %d:\n", i + 1);

        printf("  Program: ");

        for (int j = 0; j < cmds[i].argc; j++)
            printf("%s ", cmds[i].args[j]);

        printf("\n");

        if (cmds[i].input_file)
            printf("  stdin  <- %s\n", cmds[i].input_file);

        if (cmds[i].output_file) {
            if (cmds[i].append_output)
                printf("  stdout -> %s (append)\n",
                       cmds[i].output_file);
            else
                printf("  stdout -> %s (overwrite)\n",
                       cmds[i].output_file);
        }

        if (cmds[i].error_file) {
            if (cmds[i].append_error)
                printf("  stderr -> %s (append)\n",
                       cmds[i].error_file);
            else
                printf("  stderr -> %s (overwrite)\n",
                       cmds[i].error_file);
        }

        if (cmds[i].stderr_to_stdout)
            printf("  stderr -> stdout\n");

        if (i < count - 1)
            printf("  stdout | pipe |-> Command %d\n", i + 2);
    }

    printf("\n====================================\n");
}


/* Apply redirections */
void apply_redirections(Command *cmd)
{
    int fd;

    /* stdin */
    if (cmd->input_file) {

        fd = open(cmd->input_file, O_RDONLY);

        if (fd < 0) {
            perror("Input redirection");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    /* stdout */
    if (cmd->output_file) {

        int flags = O_WRONLY | O_CREAT;

        if (cmd->append_output)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;

        fd = open(cmd->output_file, flags, 0644);

        if (fd < 0) {
            perror("Output redirection");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    /* stderr */
    if (cmd->error_file) {

        int flags = O_WRONLY | O_CREAT;

        if (cmd->append_error)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;

        fd = open(cmd->error_file, flags, 0644);

        if (fd < 0) {
            perror("Error redirection");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    /*
     * Important:
     * 2>&1 duplicates the CURRENT stdout descriptor.
     *
     * Therefore:
     *   >out 2>&1
     *
     * sends both stdout and stderr to out.
     *
     * Whereas:
     *   2>&1 >out
     *
     * makes stderr continue pointing to the original stdout.
     */

    if (cmd->stderr_to_stdout) {
        dup2(STDOUT_FILENO, STDERR_FILENO);
    }
}


/* Execute pipeline */
void execute_pipeline(Command *cmds, int count)
{
    int pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];

    for (int i = 0; i < count - 1; i++) {

        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < count; i++) {

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            return;
        }

        if (pids[i] == 0) {

            /*
             * If not first command,
             * connect previous pipe to stdin.
             */
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            /*
             * If not last command,
             * connect stdout to next pipe.
             */
            if (i < count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            /* Close all pipe descriptors */
            for (int j = 0; j < count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /* Apply command-specific redirections */
            apply_redirections(&cmds[i]);

            execvp(cmds[i].args[0], cmds[i].args);

            fprintf(stderr,
                    "Execution failed for %s: %s\n",
                    cmds[i].args[0],
                    strerror(errno));

            exit(EXIT_FAILURE);
        }
    }

    /* Parent closes pipes */
    for (int i = 0; i < count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Wait for all processes */
    for (int i = 0; i < count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}


/* Parse complete pipeline */
int parse_pipeline(char *line, Command *cmds)
{
    int count = 0;

    char *part = strtok(line, "|");

    while (part != NULL) {

        if (count >= MAX_CMDS) {
            printf("Error: pipeline too long\n");
            return -1;
        }

        part = trim(part);

        if (strlen(part) == 0) {
            printf("Syntax Error: empty pipeline command\n");
            return -1;
        }

        if (parse_command(part, &cmds[count]) != 0) {
            return -1;
        }

        count++;

        part = strtok(NULL, "|");
    }

    return count;
}


int main()
{
    char line[MAX_LINE];

    printf("============================================\n");
    printf("          SKILL-17 COMMAND PROCESSOR        \n");
    printf("============================================\n");

    printf("Supports:\n");
    printf("  <   input redirection\n");
    printf("  >   output redirection\n");
    printf("  >>  append output\n");
    printf("  2>  stderr redirection\n");
    printf("  2>> stderr append\n");
    printf("  2>&1 merge stderr with stdout\n");
    printf("  |   pipes\n");
    printf("  exit to quit\n\n");

    while (1) {

        printf("skill17$ ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        line[strcspn(line, "\n")] = '\0';

        if (strlen(trim(line)) == 0)
            continue;

        if (strcmp(trim(line), "exit") == 0)
            break;

        Command cmds[MAX_CMDS];

        int count = parse_pipeline(line, cmds);

        if (count <= 0) {
            printf("Invalid command.\n");
            continue;
        }

        show_plan(cmds, count);

        printf("\n[Executing...]\n");

        execute_pipeline(cmds, count);

        printf("[Execution completed]\n\n");
    }

    printf("Skill-17 terminated.\n");

    return 0;
}
