/*
 * Skill-23: Module Integration and Runtime Error Handling
 *
 * Objective 1:
 * Connect Modules, Validate Interfaces, Test End-to-End Execution,
 * Handle Errors, Improve Reliability, Refactor Components.
 *
 * Objective 2:
 * Detect Runtime Errors, Generate Messages, Log Failures,
 * Handle Invalid Syntax, Recover Gracefully, Test Error Scenarios.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 32

#define ERROR_LOG "runtime_errors.log"


/* ============================================================
 * MODULE 1: Error Management
 * ============================================================
 */

typedef enum {
    ERROR_NONE,
    ERROR_INVALID_SYNTAX,
    ERROR_COMMAND_NOT_FOUND,
    ERROR_FORK_FAILED,
    ERROR_EXEC_FAILED,
    ERROR_WAIT_FAILED,
    ERROR_RUNTIME_FAILURE
} ErrorType;


const char *error_name(ErrorType type)
{
    switch (type) {

        case ERROR_NONE:
            return "NO_ERROR";

        case ERROR_INVALID_SYNTAX:
            return "INVALID_SYNTAX";

        case ERROR_COMMAND_NOT_FOUND:
            return "COMMAND_NOT_FOUND";

        case ERROR_FORK_FAILED:
            return "FORK_FAILED";

        case ERROR_EXEC_FAILED:
            return "EXEC_FAILED";

        case ERROR_WAIT_FAILED:
            return "WAIT_FAILED";

        case ERROR_RUNTIME_FAILURE:
            return "RUNTIME_FAILURE";

        default:
            return "UNKNOWN_ERROR";
    }
}


/*
 * Generate an error message and log it.
 */
void log_error(ErrorType type,
               const char *message)
{
    FILE *file;

    time_t current_time;
    struct tm *time_info;

    current_time = time(NULL);
    time_info = localtime(&current_time);

    printf("\n[ERROR] %s: %s\n",
           error_name(type),
           message);

    file = fopen(ERROR_LOG, "a");

    if (file == NULL) {

        fprintf(stderr,
                "[WARNING] Could not open error log.\n");

        return;
    }

    if (time_info != NULL) {

        fprintf(file,
                "[%04d-%02d-%02d %02d:%02d:%02d] ",
                time_info->tm_year + 1900,
                time_info->tm_mon + 1,
                time_info->tm_mday,
                time_info->tm_hour,
                time_info->tm_min,
                time_info->tm_sec);
    }

    fprintf(file,
            "%s: %s\n",
            error_name(type),
            message);

    fclose(file);
}


/*
 * Display last logged errors.
 */
void show_error_log(void)
{
    FILE *file;
    char line[MAX_LINE];

    file = fopen(ERROR_LOG, "r");

    if (file == NULL) {

        printf("\nNo runtime error log exists yet.\n");

        return;
    }

    printf("\n");
    printf("============================================================\n");
    printf("                    ERROR LOG\n");
    printf("============================================================\n");

    while (fgets(line,
                  sizeof(line),
                  file) != NULL) {

        printf("%s", line);
    }

    printf("============================================================\n");

    fclose(file);
}


/* ============================================================
 * MODULE 2: Job Management
 * ============================================================
 */

typedef enum {
    JOB_RUNNING,
    JOB_COMPLETED,
    JOB_FAILED
} JobState;


typedef struct {

    int job_id;

    pid_t pid;

    char command[MAX_LINE];

    JobState state;

    int exit_status;

} Job;


Job job_table[MAX_JOBS];

int job_count = 0;

int next_job_id = 1;


const char *job_state_name(JobState state)
{
    switch (state) {

        case JOB_RUNNING:
            return "RUNNING";

        case JOB_COMPLETED:
            return "COMPLETED";

        case JOB_FAILED:
            return "FAILED";

        default:
            return "UNKNOWN";
    }
}


/*
 * Validate the job module interface.
 */
int validate_job_interface(void)
{
    if (MAX_JOBS <= 0) {

        log_error(
            ERROR_RUNTIME_FAILURE,
            "Invalid job table size."
        );

        return 0;
    }

    if (job_count < 0 ||
        job_count > MAX_JOBS) {

        log_error(
            ERROR_RUNTIME_FAILURE,
            "Invalid job count."
        );

        return 0;
    }

    return 1;
}


/*
 * Add a job to the job table.
 */
int add_job(pid_t pid,
            const char *command)
{
    if (!validate_job_interface())
        return -1;

    if (job_count >= MAX_JOBS) {

        log_error(
            ERROR_RUNTIME_FAILURE,
            "Job table is full."
        );

        return -1;
    }

    job_table[job_count].job_id =
        next_job_id++;

    job_table[job_count].pid = pid;

    strncpy(
        job_table[job_count].command,
        command,
        MAX_LINE - 1
    );

    job_table[job_count]
        .command[MAX_LINE - 1] = '\0';

    job_table[job_count].state =
        JOB_RUNNING;

    job_table[job_count].exit_status = 0;

    return job_count++;
}


/*
 * Find job by ID.
 */
int find_job(int job_id)
{
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].job_id == job_id)
            return i;
    }

    return -1;
}


/*
 * Update job states.
 */
void update_jobs(void)
{
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].state != JOB_RUNNING)
            continue;

        int status;

        pid_t result =
            waitpid(
                job_table[i].pid,
                &status,
                WNOHANG
            );

        if (result == 0)
            continue;

        if (result == -1) {

            if (errno == ECHILD)
                continue;

            log_error(
                ERROR_WAIT_FAILED,
                strerror(errno)
            );

            continue;
        }

        if (WIFEXITED(status)) {

            job_table[i].exit_status =
                WEXITSTATUS(status);

            if (WEXITSTATUS(status) == 0) {

                job_table[i].state =
                    JOB_COMPLETED;
            }
            else {

                job_table[i].state =
                    JOB_FAILED;
            }
        }
        else if (WIFSIGNALED(status)) {

            job_table[i].state =
                JOB_FAILED;

            job_table[i].exit_status =
                128 + WTERMSIG(status);
        }
    }
}


/*
 * List jobs.
 */
void list_jobs(void)
{
    update_jobs();

    printf("\n");
    printf("============================================================\n");
    printf("                       JOB TABLE\n");
    printf("============================================================\n");

    if (job_count == 0) {

        printf("No jobs available.\n");

        printf("============================================================\n");

        return;
    }

    printf("%-5s %-8s %-14s %-12s %s\n",
           "ID",
           "PID",
           "STATE",
           "EXIT",
           "COMMAND");

    printf("------------------------------------------------------------\n");

    for (int i = 0;
         i < job_count;
         i++) {

        printf(
            "%-5d %-8d %-14s %-12d %s\n",
            job_table[i].job_id,
            job_table[i].pid,
            job_state_name(
                job_table[i].state
            ),
            job_table[i].exit_status,
            job_table[i].command
        );
    }

    printf("============================================================\n");
}


/* ============================================================
 * MODULE 3: Command Parser
 * ============================================================
 */


/*
 * Validate command syntax.
 */
int validate_syntax(char *command)
{
    int length;

    if (command == NULL) {

        log_error(
            ERROR_INVALID_SYNTAX,
            "Command is NULL."
        );

        return 0;
    }

    length = strlen(command);

    if (length == 0) {

        log_error(
            ERROR_INVALID_SYNTAX,
            "Empty command."
        );

        return 0;
    }

    /*
     * Detect invalid standalone pipe.
     */
    if (strstr(command, "|") != NULL) {

        log_error(
            ERROR_INVALID_SYNTAX,
            "Pipes are not supported by this module."
        );

        return 0;
    }

    /*
     * Detect duplicate background symbols.
     */
    if (strstr(command, "&&") != NULL) {

        log_error(
            ERROR_INVALID_SYNTAX,
            "Unsupported command operator."
        );

        return 0;
    }

    return 1;
}


/*
 * Parse command arguments.
 */
int parse_arguments(char *command,
                    char *args[])
{
    int argc = 0;

    char *token;

    token = strtok(
        command,
        " \t\n"
    );

    while (token != NULL) {

        if (argc >= MAX_ARGS - 1) {

            log_error(
                ERROR_INVALID_SYNTAX,
                "Too many command arguments."
            );

            return -1;
        }

        args[argc++] = token;

        token = strtok(
            NULL,
            " \t\n"
        );
    }

    args[argc] = NULL;

    return argc;
}


/*
 * Check background command.
 */
int is_background_command(char *command)
{
    int length;

    length = strlen(command);

    while (length > 0 &&
           (command[length - 1] == ' ' ||
            command[length - 1] == '\t')) {

        length--;
    }

    return (
        length > 0 &&
        command[length - 1] == '&'
    );
}


/*
 * Remove background symbol.
 */
void remove_background_symbol(char *command)
{
    int length;

    length = strlen(command);

    while (length > 0 &&
           (command[length - 1] == ' ' ||
            command[length - 1] == '\t')) {

        command[--length] = '\0';
    }

    if (length > 0 &&
        command[length - 1] == '&') {

        command[--length] = '\0';

        while (length > 0 &&
               (command[length - 1] == ' ' ||
                command[length - 1] == '\t')) {

            command[--length] = '\0';
        }
    }
}


/* ============================================================
 * MODULE 4: Command Execution
 * ============================================================
 */


/*
 * Execute a foreground command.
 */
int execute_foreground(char *command)
{
    char command_copy[MAX_LINE];

    char *args[MAX_ARGS];

    pid_t pid;

    int argc;

    int status;


    if (!validate_syntax(command))
        return -1;


    strncpy(
        command_copy,
        command,
        MAX_LINE - 1
    );

    command_copy[MAX_LINE - 1] = '\0';


    argc =
        parse_arguments(
            command_copy,
            args
        );


    if (argc <= 0)
        return -1;


    pid = fork();


    if (pid < 0) {

        log_error(
            ERROR_FORK_FAILED,
            strerror(errno)
        );

        return -1;
    }


    if (pid == 0) {

        /*
         * Child process.
         */
        execvp(
            args[0],
            args
        );


        /*
         * If execvp fails,
         * generate an error.
         */
        fprintf(
            stderr,
            "[ERROR] EXEC_FAILED: %s\n",
            strerror(errno)
        );


        exit(127);
    }


    /*
     * Parent waits for child.
     */
    while (1) {

        if (waitpid(
                pid,
                &status,
                0
            ) == -1) {

            if (errno == EINTR)
                continue;

            log_error(
                ERROR_WAIT_FAILED,
                strerror(errno)
            );

            return -1;
        }

        break;
    }


    if (WIFEXITED(status)) {

        int exit_code =
            WEXITSTATUS(status);


        if (exit_code == 0) {

            printf(
                "Execution successful. Exit status = 0\n"
            );

            return 0;
        }


        /*
         * Command executed but returned
         * an error.
         */
        char message[MAX_LINE];

        snprintf(
            message,
            sizeof(message),
            "Command '%s' returned exit status %d.",
            command,
            exit_code
        );

        log_error(
            ERROR_RUNTIME_FAILURE,
            message
        );

        return exit_code;
    }


    if (WIFSIGNALED(status)) {

        char message[MAX_LINE];

        snprintf(
            message,
            sizeof(message),
            "Command '%s' terminated by signal %d.",
            command,
            WTERMSIG(status)
        );

        log_error(
            ERROR_RUNTIME_FAILURE,
            message
        );

        return -1;
    }


    return -1;
}


/*
 * Execute a background command.
 */
int execute_background(char *command)
{
    char command_copy[MAX_LINE];

    char *args[MAX_ARGS];

    pid_t pid;

    int argc;


    if (!validate_syntax(command))
        return -1;


    strncpy(
        command_copy,
        command,
        MAX_LINE - 1
    );

    command_copy[MAX_LINE - 1] = '\0';


    argc =
        parse_arguments(
            command_copy,
            args
        );


    if (argc <= 0)
        return -1;


    pid = fork();


    if (pid < 0) {

        log_error(
            ERROR_FORK_FAILED,
            strerror(errno)
        );

        return -1;
    }


    if (pid == 0) {

        execvp(
            args[0],
            args
        );


        /*
         * Child cannot return to shell module
         * if exec fails.
         */
        fprintf(
            stderr,
            "[ERROR] EXEC_FAILED: %s\n",
            strerror(errno)
        );

        exit(127);
    }


    /*
     * Parent does not block.
     */
    int index =
        add_job(
            pid,
            command
        );


    if (index < 0) {

        /*
         * Job could not be stored,
         * terminate child to avoid orphaning.
         */
        kill(
            pid,
            SIGTERM
        );

        return -1;
    }


    printf(
        "[%d] PID=%d RUNNING\n",
        job_table[index].job_id,
        pid
    );


    return 0;
}


/* ============================================================
 * MODULE 5: Interface Validation
 * ============================================================
 */


/*
 * Validate connections between modules.
 */
int validate_modules(void)
{
    int valid = 1;

    printf("\n");
    printf("============================================================\n");
    printf("                 MODULE INTERFACE TEST\n");
    printf("============================================================\n");


    /*
     * Error module interface.
     */
    printf(
        "Error Module       : %s\n",
        log_error != NULL ?
        "CONNECTED" :
        "FAILED"
    );


    /*
     * Parser module interface.
     */
    printf(
        "Parser Module      : %s\n",
        parse_arguments != NULL ?
        "CONNECTED" :
        "FAILED"
    );


    /*
     * Job module interface.
     */
    if (validate_job_interface()) {

        printf(
            "Job Module         : CONNECTED\n"
        );
    }
    else {

        printf(
            "Job Module         : FAILED\n"
        );

        valid = 0;
    }


    /*
     * Executor module interface.
     */
    printf(
        "Execution Module   : %s\n",
        execute_foreground != NULL ?
        "CONNECTED" :
        "FAILED"
    );


    if (valid) {

        printf(
            "\nInterface validation: PASSED\n"
        );
    }
    else {

        printf(
            "\nInterface validation: FAILED\n"
        );
    }


    printf("============================================================\n");

    return valid;
}


/* ============================================================
 * MODULE 6: Error Scenario Testing
 * ============================================================
 */


/*
 * Test invalid syntax.
 */
void test_invalid_syntax(void)
{
    char command[MAX_LINE];

    printf("\n");
    printf("========== TEST 1: INVALID SYNTAX ==========\n");

    strcpy(
        command,
        ""
    );

    printf("Testing empty command...\n");

    validate_syntax(command);


    strcpy(
        command,
        "invalid && command"
    );

    printf("Testing unsupported operator...\n");

    validate_syntax(command);


    strcpy(
        command,
        "echo hello | grep hello"
    );

    printf("Testing unsupported pipe...\n");

    validate_syntax(command);

    printf("=============================================\n");
}


/*
 * Test command-not-found scenario.
 */
void test_command_error(void)
{
    char command[MAX_LINE];

    printf("\n");
    printf("========== TEST 2: COMMAND ERROR ==========\n");

    strcpy(
        command,
        "command_that_does_not_exist"
    );

    printf(
        "Executing invalid command...\n"
    );

    execute_foreground(command);

    printf("===========================================\n");
}


/*
 * Test successful end-to-end execution.
 */
void test_successful_execution(void)
{
    char command[MAX_LINE];

    printf("\n");
    printf("========== TEST 3: END-TO-END ==========\n");

    strcpy(
        command,
        "echo Skill-23-End-to-End-Success"
    );

    printf(
        "Executing valid command...\n"
    );

    execute_foreground(command);

    printf("=========================================\n");
}


/*
 * Test runtime failure.
 */
void test_runtime_failure(void)
{
    char command[MAX_LINE];

    printf("\n");
    printf("========== TEST 4: RUNTIME ERROR ==========\n");

    strcpy(
        command,
        "sh -c exit 5"
    );

    printf(
        "Executing command that returns error status...\n"
    );

    execute_foreground(command);

    printf("============================================\n");
}


/*
 * Run all error tests.
 */
void run_tests(void)
{
    test_invalid_syntax();

    test_command_error();

    test_successful_execution();

    test_runtime_failure();

    printf("\n");
    printf("All Skill-23 test scenarios completed.\n");
}


/* ============================================================
 * MODULE 7: Help
 * ============================================================
 */

void show_help(void)
{
    printf("\n");
    printf("Available commands:\n");

    printf("  command          Execute foreground command\n");
    printf("  command &        Execute background command\n");

    printf("  jobs             List background jobs\n");

    printf("  modules          Validate module interfaces\n");

    printf("  test             Run error and integration tests\n");

    printf("  errors           Display runtime error log\n");

    printf("  clear-errors     Clear error log\n");

    printf("  help             Display help\n");

    printf("  exit             Exit program\n");

    printf("\n");
}


/* ============================================================
 * MAIN MODULE
 * ============================================================
 */

int main(void)
{
    char line[MAX_LINE];


    printf("============================================================\n");
    printf("       SKILL-23 MODULE INTEGRATION & ERROR MANAGER\n");
    printf("============================================================\n");


    /*
     * Validate module interfaces before execution.
     */
    validate_modules();

    show_help();


    while (1) {

        /*
         * Update background jobs.
         */
        update_jobs();


        printf("skill23$ ");

        fflush(stdout);


        /*
         * Read user input.
         */
        if (fgets(
                line,
                sizeof(line),
                stdin
            ) == NULL) {

            break;
        }


        line[strcspn(
            line,
            "\n"
        )] = '\0';


        /*
         * Empty input.
         */
        if (strlen(line) == 0) {

            log_error(
                ERROR_INVALID_SYNTAX,
                "Empty command entered."
            );

            continue;
        }


        /*
         * Exit.
         */
        if (strcmp(
                line,
                "exit"
            ) == 0) {

            break;
        }


        /*
         * Help.
         */
        if (strcmp(
                line,
                "help"
            ) == 0) {

            show_help();

            continue;
        }


        /*
         * List jobs.
         */
        if (strcmp(
                line,
                "jobs"
            ) == 0) {

            list_jobs();

            continue;
        }


        /*
         * Validate modules.
         */
        if (strcmp(
                line,
                "modules"
            ) == 0) {

            validate_modules();

            continue;
        }


        /*
         * Show errors.
         */
        if (strcmp(
                line,
                "errors"
            ) == 0) {

            show_error_log();

            continue;
        }


        /*
         * Clear error log.
         */
        if (strcmp(
                line,
                "clear-errors"
            ) == 0) {

            if (remove(ERROR_LOG) == 0) {

                printf(
                    "Error log cleared.\n"
                );
            }
            else {

                if (errno == ENOENT) {

                    printf(
                        "Error log is already empty.\n"
                    );
                }
                else {

                    log_error(
                        ERROR_RUNTIME_FAILURE,
                        strerror(errno)
                    );
                }
            }

            continue;
        }


        /*
         * Run built-in tests.
         */
        if (strcmp(
                line,
                "test"
            ) == 0) {

            run_tests();

            continue;
        }


        /*
         * Detect background command.
         */
        if (is_background_command(line)) {

            remove_background_symbol(line);

            if (strlen(line) == 0) {

                log_error(
                    ERROR_INVALID_SYNTAX,
                    "Background operator without command."
                );

                continue;
            }

            execute_background(line);

            continue;
        }


        /*
         * Execute foreground command.
         */
        execute_foreground(line);
    }


    /*
     * Terminate remaining jobs.
     */
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].state ==
            JOB_RUNNING) {

            kill(
                job_table[i].pid,
                SIGTERM
            );
        }
    }


    /*
     * Reap remaining jobs.
     */
    for (int i = 0;
         i < job_count;
         i++) {

        waitpid(
            job_table[i].pid,
            NULL,
            0
        );
    }


    printf("\nSkill-23 terminated.\n");

    return 0;
}
