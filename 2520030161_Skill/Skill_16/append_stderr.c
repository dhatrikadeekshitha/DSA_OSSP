#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT 4096
#define MAX_FILE 1024


/* =========================================================
   SKILL_16
   APPEND AND STDERR REDIRECTION
   ========================================================= */


/* =========================================================
   TRIM SPACES
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
   SAVE STDOUT
   ========================================================= */

int save_stdout(void)
{
    int saved = dup(STDOUT_FILENO);

    if (saved < 0)
    {
        perror("dup stdout");
    }

    return saved;
}


/* =========================================================
   SAVE STDERR
   ========================================================= */

int save_stderr(void)
{
    int saved = dup(STDERR_FILENO);

    if (saved < 0)
    {
        perror("dup stderr");
    }

    return saved;
}


/* =========================================================
   RESTORE STDOUT
   ========================================================= */

int restore_stdout(int saved)
{
    if (dup2(saved,
             STDOUT_FILENO) < 0)
    {
        perror("restore stdout");

        close(saved);

        return 0;
    }

    close(saved);

    return 1;
}


/* =========================================================
   RESTORE STDERR
   ========================================================= */

int restore_stderr(int saved)
{
    if (dup2(saved,
             STDERR_FILENO) < 0)
    {
        /*
         * stderr itself may be redirected,
         * so avoid relying on it here.
         */

        close(saved);

        return 0;
    }

    close(saved);

    return 1;
}


/* =========================================================
   PARSE APPEND OPERATOR
   ========================================================= */

int parse_append(
    const char *input,
    char *command,
    size_t command_size,
    char *filename,
    size_t filename_size)
{
    const char *position = strstr(input, ">>");

    if (position == NULL)
    {
        return 0;
    }

    size_t command_length =
        (size_t)(position - input);

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
        position + 2;


    while (*file_start == ' ' ||
           *file_start == '\t')
    {
        file_start++;
    }


    if (*file_start == '\0')
    {
        fprintf(stderr,
                "Error: append output file is missing.\n");

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
   PARSE STDERR REDIRECTION
   ========================================================= */

int parse_stderr(
    const char *input,
    char *command,
    size_t command_size,
    char *filename,
    size_t filename_size)
{
    const char *position = strstr(input, "2>");

    if (position == NULL)
    {
        return 0;
    }

    size_t command_length =
        (size_t)(position - input);

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
        position + 2;


    while (*file_start == ' ' ||
           *file_start == '\t')
    {
        file_start++;
    }


    if (*file_start == '\0')
    {
        fprintf(stderr,
                "Error: stderr output file is missing.\n");

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
   REDIRECT STDOUT TO APPEND FILE
   ========================================================= */

int redirect_stdout_append(
    const char *filename)
{
    int fd = open(
        filename,
        O_WRONLY |
        O_CREAT |
        O_APPEND,
        0644);


    if (fd < 0)
    {
        fprintf(stderr,
                "Append redirection failed for '%s': %s\n",
                filename,
                strerror(errno));

        return 0;
    }


    if (dup2(fd,
             STDOUT_FILENO) < 0)
    {
        fprintf(stderr,
                "dup2 stdout failed: %s\n",
                strerror(errno));

        close(fd);

        return 0;
    }


    close(fd);

    return 1;
}


/* =========================================================
   REDIRECT STDERR TO FILE
   ========================================================= */

int redirect_stderr(
    const char *filename)
{
    int fd = open(
        filename,
        O_WRONLY |
        O_CREAT |
        O_TRUNC,
        0644);


    if (fd < 0)
    {
        fprintf(stderr,
                "stderr redirection failed for '%s': %s\n",
                filename,
                strerror(errno));

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

    return 1;
}


/* =========================================================
   DISPLAY FILE CONTENT
   ========================================================= */

void display_file(
    const char *filename)
{
    FILE *file =
        fopen(filename, "r");


    if (file == NULL)
    {
        fprintf(stderr,
                "Cannot read '%s': %s\n",
                filename,
                strerror(errno));

        return;
    }


    printf("\n========== FILE CONTENT ==========\n");

    printf("File: %s\n\n",
           filename);


    char buffer[MAX_INPUT];

    while (fgets(buffer,
                 sizeof(buffer),
                 file) != NULL)
    {
        printf("%s",
               buffer);
    }


    printf("\n==================================\n");


    fclose(file);
}


/* =========================================================
   VERIFY FILE
   ========================================================= */

void verify_file(
    const char *filename)
{
    struct stat info;


    if (stat(filename,
             &info) < 0)
    {
        fprintf(stderr,
                "Verification failed for '%s': %s\n",
                filename,
                strerror(errno));

        return;
    }


    printf("\n========== FILE VERIFICATION ==========\n");

    printf("File        : %s\n",
           filename);

    printf("Size        : %ld bytes\n",
           (long)info.st_size);

    printf("Permissions : %o\n",
           info.st_mode & 0777);

    printf("Status      : File exists\n");

    printf("=======================================\n");
}


/* =========================================================
   TEST APPEND REDIRECTION
   ========================================================= */

void test_append(
    const char *filename)
{
    printf("\n--- APPEND REDIRECTION TEST ---\n");

    int saved_stdout =
        save_stdout();


    if (saved_stdout < 0)
    {
        return;
    }


    if (!redirect_stdout_append(filename))
    {
        restore_stdout(saved_stdout);

        return;
    }


    /*
     * These lines are appended.
     */

    printf("Append test line 1.\n");

    printf("Append test line 2.\n");

    printf("Skill_16 verified append redirection.\n");


    fflush(stdout);


    /*
     * Restore stdout.
     */

    if (!restore_stdout(saved_stdout))
    {
        return;
    }


    printf("stdout restored successfully.\n");


    verify_file(filename);

    display_file(filename);
}


/* =========================================================
   TEST STDERR REDIRECTION
   ========================================================= */

void test_stderr(
    const char *filename)
{
    printf("\n--- STDERR REDIRECTION TEST ---\n");


    int saved_stderr =
        save_stderr();


    if (saved_stderr < 0)
    {
        return;
    }


    if (!redirect_stderr(filename))
    {
        restore_stderr(saved_stderr);

        return;
    }


    /*
     * Generate deliberate error output.
     */

    fprintf(stderr,
            "Skill_16 test error: this message goes to stderr.\n");

    fprintf(stderr,
            "Error test: requested file does not exist.\n");


    fflush(stderr);


    /*
     * Restore stderr.
     */

    if (!restore_stderr(saved_stderr))
    {
        return;
    }


    printf("stderr restored successfully.\n");


    verify_file(filename);

    display_file(filename);
}


/* =========================================================
   TEST REAL COMMAND FAILURE
   ========================================================= */

void test_command_failure(
    const char *error_file)
{
    printf("\n--- COMMAND ERROR CAPTURE TEST ---\n");


    int saved_stderr =
        save_stderr();


    if (saved_stderr < 0)
    {
        return;
    }


    if (!redirect_stderr(error_file))
    {
        restore_stderr(saved_stderr);

        return;
    }


    /*
     * Execute a command that should fail.
     *
     * The shell command redirects its
     * stderr into the selected file.
     */

    int status =
        system("ls /file_that_does_not_exist");


    fflush(stderr);


    if (!restore_stderr(saved_stderr))
    {
        return;
    }


    if (status == -1)
    {
        printf("Command execution failed.\n");
    }
    else if (WIFEXITED(status))
    {
        printf("Command exit status: %d\n",
               WEXITSTATUS(status));
    }


    printf("stderr restored successfully.\n");


    verify_file(error_file);

    display_file(error_file);
}


/* =========================================================
   PROCESS INPUT
   ========================================================= */

void process_input(
    char *input)
{
    trim_spaces(input);


    if (strlen(input) == 0)
    {
        return;
    }


    /* =========================================
       APPEND REDIRECTION
       ========================================= */

    if (strstr(input, ">>") != NULL)
    {
        char command[MAX_INPUT];

        char filename[MAX_FILE];


        int result =
            parse_append(
                input,
                command,
                sizeof(command),
                filename,
                sizeof(filename));


        if (result <= 0)
        {
            return;
        }


        printf("\n========== APPEND REDIRECTION ==========\n");

        printf("Command : %s\n",
               command);

        printf("File    : %s\n",
               filename);

        printf("Mode    : O_APPEND\n");

        printf("========================================\n");


        test_append(filename);

        return;
    }


    /* =========================================
       STDERR REDIRECTION
       ========================================= */

    if (strstr(input, "2>") != NULL)
    {
        char command[MAX_INPUT];

        char filename[MAX_FILE];


        int result =
            parse_stderr(
                input,
                command,
                sizeof(command),
                filename,
                sizeof(filename));


        if (result <= 0)
        {
            return;
        }


        printf("\n========== STDERR REDIRECTION ==========\n");

        printf("Command : %s\n",
               command);

        printf("File    : %s\n",
               filename);

        printf("Stream  : stderr (2)\n");

        printf("========================================\n");


        test_stderr(filename);

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

        return;
    }


    if (strcmp(input, "date") == 0)
    {
        system("date");

        return;
    }


    if (strcmp(input, "whoami") == 0)
    {
        system("whoami");

        return;
    }


    printf("Command entered: %s\n",
           input);
}


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    char input[MAX_INPUT];


    printf("===============================================\n");

    printf(" SKILL_16 - APPEND & STDERR REDIRECTION\n");

    printf("===============================================\n");


    printf("\nSupported syntax:\n");

    printf("  command >> output.txt\n");

    printf("  command 2> error.txt\n");


    printf("\nExamples:\n");

    printf("  echo first >> output.txt\n");

    printf("  echo second >> output.txt\n");

    printf("  ls missing_file 2> error.txt\n");


    printf("\nType exit to quit.\n");


    while (1)
    {
        printf("\nskill16> ");

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


    printf("\nSkill_16 exited successfully.\n");


    return 0;
}
