#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_INPUT 4096

/*
 * SKILL_15
 *
 * Input Redirection:
 *      command < input.txt
 *
 * Output Redirection:
 *      command > output.txt
 *
 * Demonstrates:
 * - Parsing input redirection
 * - Parsing output redirection
 * - Opening files
 * - Redirecting stdin
 * - Redirecting stdout
 * - Handling missing files
 * - Handling permission errors
 * - Restoring stdin/stdout
 * - Verifying output files
 */


/* =========================================================
   REMOVE TRAILING SPACES
   ========================================================= */

void trim_spaces(char *text)
{
    size_t length = strlen(text);

    while (length > 0 &&
           (text[length - 1] == ' ' ||
            text[length - 1] == '\t'))
    {
        text[length - 1] = '\0';
        length--;
    }
}


/* =========================================================
   PARSE INPUT REDIRECTION
   ========================================================= */

int parse_input_redirection(
    const char *input,
    char *command,
    size_t command_size,
    char *filename,
    size_t filename_size)
{
    const char *operator_pos = strchr(input, '<');

    if (operator_pos == NULL)
    {
        return 0;
    }

    size_t command_length =
        (size_t)(operator_pos - input);

    if (command_length >= command_size)
    {
        fprintf(stderr,
                "Error: command is too long.\n");

        return -1;
    }

    memcpy(command,
           input,
           command_length);

    command[command_length] = '\0';

    const char *file_start =
        operator_pos + 1;

    while (*file_start == ' ' ||
           *file_start == '\t')
    {
        file_start++;
    }

    if (*file_start == '\0')
    {
        fprintf(stderr,
                "Error: input file is missing.\n");

        return -1;
    }

    strncpy(filename,
            file_start,
            filename_size - 1);

    filename[filename_size - 1] = '\0';

    trim_spaces(command);
    trim_spaces(filename);

    return 1;
}


/* =========================================================
   PARSE OUTPUT REDIRECTION
   ========================================================= */

int parse_output_redirection(
    const char *input,
    char *command,
    size_t command_size,
    char *filename,
    size_t filename_size)
{
    const char *operator_pos = strchr(input, '>');

    if (operator_pos == NULL)
    {
        return 0;
    }

    size_t command_length =
        (size_t)(operator_pos - input);

    if (command_length >= command_size)
    {
        fprintf(stderr,
                "Error: command is too long.\n");

        return -1;
    }

    memcpy(command,
           input,
           command_length);

    command[command_length] = '\0';

    const char *file_start =
        operator_pos + 1;

    while (*file_start == ' ' ||
           *file_start == '\t')
    {
        file_start++;
    }

    if (*file_start == '\0')
    {
        fprintf(stderr,
                "Error: output file is missing.\n");

        return -1;
    }

    strncpy(filename,
            file_start,
            filename_size - 1);

    filename[filename_size - 1] = '\0';

    trim_spaces(command);
    trim_spaces(filename);

    return 1;
}


/* =========================================================
   SAVE STDIN
   ========================================================= */

int save_stdin(void)
{
    int saved_stdin = dup(STDIN_FILENO);

    if (saved_stdin < 0)
    {
        perror("dup stdin");
    }

    return saved_stdin;
}


/* =========================================================
   SAVE STDOUT
   ========================================================= */

int save_stdout(void)
{
    int saved_stdout = dup(STDOUT_FILENO);

    if (saved_stdout < 0)
    {
        perror("dup stdout");
    }

    return saved_stdout;
}


/* =========================================================
   RESTORE STDIN
   ========================================================= */

int restore_stdin(int saved_stdin)
{
    if (dup2(saved_stdin,
             STDIN_FILENO) < 0)
    {
        perror("restore stdin");

        close(saved_stdin);

        return 0;
    }

    close(saved_stdin);

    return 1;
}


/* =========================================================
   RESTORE STDOUT
   ========================================================= */

int restore_stdout(int saved_stdout)
{
    if (dup2(saved_stdout,
             STDOUT_FILENO) < 0)
    {
        perror("restore stdout");

        close(saved_stdout);

        return 0;
    }

    close(saved_stdout);

    return 1;
}


/* =========================================================
   REDIRECT STDIN
   ========================================================= */

int redirect_input(const char *filename)
{
    int fd = open(filename,
                  O_RDONLY);

    if (fd < 0)
    {
        fprintf(stderr,
                "Input redirection failed for '%s': %s\n",
                filename,
                strerror(errno));

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

    return 1;
}


/* =========================================================
   REDIRECT STDOUT
   ========================================================= */

int redirect_output(const char *filename)
{
    int fd = open(filename,
                  O_WRONLY |
                  O_CREAT |
                  O_TRUNC,
                  0644);

    if (fd < 0)
    {
        fprintf(stderr,
                "Output redirection failed for '%s': %s\n",
                filename,
                strerror(errno));

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

    return 1;
}


/* =========================================================
   VERIFY OUTPUT FILE
   ========================================================= */

int verify_file(const char *filename)
{
    struct stat file_info;

    if (stat(filename,
             &file_info) < 0)
    {
        fprintf(stderr,
                "Verification failed for '%s': %s\n",
                filename,
                strerror(errno));

        return 0;
    }

    printf("\n========== FILE VERIFICATION ==========\n");

    printf("File        : %s\n",
           filename);

    printf("Size        : %ld bytes\n",
           (long)file_info.st_size);

    printf("Permissions : %o\n",
           file_info.st_mode & 0777);

    printf("Status      : File exists\n");

    printf("=======================================\n");

    return 1;
}


/* =========================================================
   TEST INPUT REDIRECTION
   ========================================================= */

void test_input_redirection(
    const char *filename)
{
    printf("\n--- Testing Input Redirection ---\n");

    int saved_stdin = save_stdin();

    if (saved_stdin < 0)
    {
        return;
    }

    if (!redirect_input(filename))
    {
        restore_stdin(saved_stdin);

        return;
    }

    /*
     * stdin now comes from the file.
     */

    char buffer[MAX_INPUT];

    int line_count = 0;

    while (fgets(buffer,
                 sizeof(buffer),
                 stdin) != NULL)
    {
        printf("INPUT: %s",
               buffer);

        line_count++;
    }

    /*
     * Restore stdin.
     */

    if (!restore_stdin(saved_stdin))
    {
        fprintf(stderr,
                "Warning: stdin restoration failed.\n");
    }
    else
    {
        printf("\nstdin restored successfully.\n");
    }

    printf("Lines read: %d\n",
           line_count);
}


/* =========================================================
   TEST OUTPUT REDIRECTION
   ========================================================= */

void test_output_redirection(
    const char *filename)
{
    printf("\n--- Testing Output Redirection ---\n");

    int saved_stdout = save_stdout();

    if (saved_stdout < 0)
    {
        return;
    }

    if (!redirect_output(filename))
    {
        restore_stdout(saved_stdout);

        return;
    }

    /*
     * stdout now goes to the file.
     */

    printf("Output redirection test successful.\n");

    printf("This text was redirected to the output file.\n");

    printf("Skill_15 verified stdout redirection.\n");

    fflush(stdout);

    /*
     * Restore stdout.
     */

    if (!restore_stdout(saved_stdout))
    {
        fprintf(stderr,
                "Warning: stdout restoration failed.\n");

        return;
    }

    printf("stdout restored successfully.\n");

    verify_file(filename);
}


/* =========================================================
   PROCESS INPUT
   ========================================================= */

void process_input(char *input)
{
    trim_spaces(input);

    if (strlen(input) == 0)
    {
        return;
    }


    /* =========================================
       INPUT REDIRECTION
       ========================================= */

    if (strchr(input, '<') != NULL)
    {
        char command[MAX_INPUT];

        char filename[MAX_INPUT];

        int result =
            parse_input_redirection(
                input,
                command,
                sizeof(command),
                filename,
                sizeof(filename));

        if (result <= 0)
        {
            return;
        }

        printf("\n========== INPUT REDIRECTION ==========\n");

        printf("Command : %s\n",
               command);

        printf("File    : %s\n",
               filename);

        printf("=======================================\n");

        test_input_redirection(filename);

        return;
    }


    /* =========================================
       OUTPUT REDIRECTION
       ========================================= */

    if (strchr(input, '>') != NULL)
    {
        char command[MAX_INPUT];

        char filename[MAX_INPUT];

        int result =
            parse_output_redirection(
                input,
                command,
                sizeof(command),
                filename,
                sizeof(filename));

        if (result <= 0)
        {
            return;
        }

        printf("\n========== OUTPUT REDIRECTION ==========\n");

        printf("Command : %s\n",
               command);

        printf("File    : %s\n",
               filename);

        printf("========================================\n");

        test_output_redirection(filename);

        return;
    }


    /* =========================================
       NORMAL COMMANDS
       ========================================= */

    if (strcmp(input, "pwd") == 0)
    {
        char path[MAX_INPUT];

        if (getcwd(path,
                   sizeof(path)) != NULL)
        {
            printf("%s\n",
                   path);
        }
        else
        {
            perror("getcwd");
        }
    }
    else if (strcmp(input, "date") == 0)
    {
        system("date");
    }
    else if (strcmp(input, "whoami") == 0)
    {
        system("whoami");
    }
    else
    {
        printf("Command entered: %s\n",
               input);
    }
}


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    char input[MAX_INPUT];

    printf("============================================\n");
    printf(" SKILL_15 - INPUT & OUTPUT REDIRECTION\n");
    printf("============================================\n");

    printf("\nSupported syntax:\n");
    printf("  command < input.txt\n");
    printf("  command > output.txt\n");

    printf("\nTests:\n");
    printf("  cat < input.txt\n");
    printf("  echo hello > output.txt\n");

    printf("\nType exit to quit.\n");

    while (1)
    {
        printf("\nskill15> ");

        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[
            strcspn(input, "\n")
        ] = '\0';

        trim_spaces(input);

        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        process_input(input);
    }

    printf("\nSkill_15 exited successfully.\n");

    return 0;
}
