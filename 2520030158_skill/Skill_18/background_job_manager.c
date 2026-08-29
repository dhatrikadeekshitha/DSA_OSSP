/*
 * Skill-18: Background Job Management
 *
 * Objectives:
 * 1. Detect background jobs, launch non-blocking processes,
 *    return prompt immediately, store job information,
 *    monitor execution and test background tasks.
 *
 * 2. Create job table, store process groups, track states,
 *    update status, remove completed jobs and validate
 *    job records.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 32

/* Job states */
typedef enum {
    RUNNING,
    COMPLETED,
    FAILED
} JobState;

/* Job record */
typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;
    char command[MAX_LINE];
    JobState state;
} Job;

/* Global job table */
Job job_table[MAX_JOBS];

int job_count = 0;
int next_job_id = 1;


/* Convert job state to string */
const char *get_state(JobState state)
{
    switch (state) {
        case RUNNING:
            return "RUNNING";

        case COMPLETED:
            return "COMPLETED";

        case FAILED:
            return "FAILED";

        default:
            return "UNKNOWN";
    }
}


/* Check whether command ends with '&' */
int is_background_job(char *command)
{
    int len = strlen(command);

    while (len > 0 &&
           (command[len - 1] == ' ' ||
            command[len - 1] == '\t' ||
            command[len - 1] == '\n')) {
        len--;
    }

    if (len > 0 && command[len - 1] == '&')
        return 1;

    return 0;
}


/* Remove '&' from the command */
void remove_background_symbol(char *command)
{
    int len = strlen(command);

    while (len > 0 &&
           (command[len - 1] == ' ' ||
            command[len - 1] == '\t' ||
            command[len - 1] == '\n')) {
        command[--len] = '\0';
    }

    if (len > 0 && command[len - 1] == '&') {

        command[--len] = '\0';

        while (len > 0 &&
               (command[len - 1] == ' ' ||
                command[len - 1] == '\t')) {
            command[--len] = '\0';
        }
    }
}


/* Parse command into arguments */
int parse_arguments(char *command, char *args[])
{
    int argc = 0;

    char *token = strtok(command, " \t\n");

    while (token != NULL) {

        if (argc >= MAX_ARGS - 1) {
            printf("Error: Too many arguments.\n");
            return -1;
        }

        args[argc++] = token;

        token = strtok(NULL, " \t\n");
    }

    args[argc] = NULL;

    return argc;
}


/* Add a new job to the job table */
int add_job(pid_t pid, pid_t pgid, const char *command)
{
    if (job_count >= MAX_JOBS) {
        printf("Error: Job table is full.\n");
        return -1;
    }

    job_table[job_count].job_id = next_job_id++;
    job_table[job_count].pid = pid;
    job_table[job_count].pgid = pgid;

    strncpy(job_table[job_count].command,
            command,
            MAX_LINE - 1);

    job_table[job_count].command[MAX_LINE - 1] = '\0';

    job_table[job_count].state = RUNNING;

    printf("[%d] PID=%d PGID=%d started in background\n",
           job_table[job_count].job_id,
           job_table[job_count].pid,
           job_table[job_count].pgid);

    job_count++;

    return 0;
}


/* Remove a job from the job table */
void remove_job(int index)
{
    if (index < 0 || index >= job_count)
        return;

    for (int i = index; i < job_count - 1; i++) {
        job_table[i] = job_table[i + 1];
    }

    job_count--;
}


/* Check and update background jobs */
void update_job_status(void)
{
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].state != RUNNING)
            continue;

        int status;

        /*
         * WNOHANG makes waitpid() non-blocking.
         */
        pid_t result = waitpid(
            job_table[i].pid,
            &status,
            WNOHANG
        );

        if (result == 0) {

            /* Process is still running */
            continue;
        }

        if (result == job_table[i].pid) {

            if (WIFEXITED(status) &&
                WEXITSTATUS(status) == 0) {

                job_table[i].state = COMPLETED;

                printf("\n[%d] completed: %s\n",
                       job_table[i].job_id,
                       job_table[i].command);

            } else {

                job_table[i].state = FAILED;

                printf("\n[%d] failed: %s\n",
                       job_table[i].job_id,
                       job_table[i].command);
            }
        }

        else if (result == -1) {

            if (errno == ECHILD) {
                job_table[i].state = COMPLETED;
            }
        }
    }
}


/* Display job table */
void display_jobs(void)
{
    update_job_status();

    printf("\n");
    printf("====================================================\n");
    printf("                    JOB TABLE\n");
    printf("====================================================\n");

    if (job_count == 0) {

        printf("No jobs in the job table.\n");

        printf("====================================================\n");

        return;
    }

    printf("%-5s %-8s %-8s %-12s %s\n",
           "ID",
           "PID",
           "PGID",
           "STATE",
           "COMMAND");

    printf("----------------------------------------------------\n");

    for (int i = 0; i < job_count; i++) {

        printf("%-5d %-8d %-8d %-12s %s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               get_state(job_table[i].state),
               job_table[i].command);
    }

    printf("====================================================\n");
}


/* Remove completed and failed jobs */
void cleanup_jobs(void)
{
    update_job_status();

    int removed = 0;

    for (int i = job_count - 1; i >= 0; i--) {

        if (job_table[i].state == COMPLETED ||
            job_table[i].state == FAILED) {

            remove_job(i);
            removed++;
        }
    }

    printf("Cleanup completed: %d job(s) removed.\n", removed);
}


/* Validate all job records */
void validate_jobs(void)
{
    update_job_status();

    printf("\n");
    printf("====================================================\n");
    printf("                 JOB VALIDATION\n");
    printf("====================================================\n");

    if (job_count == 0) {

        printf("Job table is empty.\n");
        printf("Validation: OK\n");

        printf("====================================================\n");

        return;
    }

    int valid = 1;

    for (int i = 0; i < job_count; i++) {

        printf("Job ID: %d\n",
               job_table[i].job_id);

        printf("PID   : %d\n",
               job_table[i].pid);

        printf("PGID  : %d\n",
               job_table[i].pgid);

        printf("State : %s\n",
               get_state(job_table[i].state));

        printf("Command: %s\n",
               job_table[i].command);

        printf("\n");

        if (job_table[i].job_id <= 0)
            valid = 0;

        if (job_table[i].pid <= 0)
            valid = 0;

        if (job_table[i].pgid <= 0)
            valid = 0;

        if (strlen(job_table[i].command) == 0)
            valid = 0;
    }

    if (valid)
        printf("Validation: ALL JOB RECORDS ARE VALID\n");
    else
        printf("Validation: INVALID JOB RECORD FOUND\n");

    printf("====================================================\n");
}


/* Launch background process */
void launch_background(char *command)
{
    char command_copy[MAX_LINE];

    strncpy(command_copy,
            command,
            MAX_LINE - 1);

    command_copy[MAX_LINE - 1] = '\0';

    char *args[MAX_ARGS];

    int argc = parse_arguments(command_copy, args);

    if (argc <= 0)
        return;

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");
        return;
    }

    if (pid == 0) {

        /*
         * Create a new process group.
         */
        if (setpgid(0, 0) == -1) {
            perror("setpgid");
            exit(EXIT_FAILURE);
        }

        /*
         * Execute the background command.
         */
        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent also sets process group.
     */
    setpgid(pid, pid);

    /*
     * Store PID and PGID in job table.
     */
    add_job(pid, pid, command);

    /*
     * Do NOT wait for background process.
     * Prompt returns immediately.
     */
}


/* Launch foreground process */
void launch_foreground(char *command)
{
    char command_copy[MAX_LINE];

    strncpy(command_copy,
            command,
            MAX_LINE - 1);

    command_copy[MAX_LINE - 1] = '\0';

    char *args[MAX_ARGS];

    int argc = parse_arguments(command_copy, args);

    if (argc <= 0)
        return;

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");
        return;
    }

    if (pid == 0) {

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Foreground process blocks until completion.
     */
    int status;

    waitpid(pid, &status, 0);
}


/* Terminate all running background jobs */
void terminate_all_jobs(void)
{
    update_job_status();

    for (int i = 0; i < job_count; i++) {

        if (job_table[i].state == RUNNING) {

            printf("Terminating Job [%d], PID=%d\n",
                   job_table[i].job_id,
                   job_table[i].pid);

            /*
             * Negative PGID sends signal to
             * the entire process group.
             */
            if (kill(-job_table[i].pgid, SIGTERM) == -1) {

                if (errno != ESRCH)
                    perror("kill");
            }
        }
    }

    /*
     * Reap remaining children.
     */
    for (int i = 0; i < job_count; i++) {

        waitpid(job_table[i].pid, NULL, 0);
    }
}


/* Display help */
void display_help(void)
{
    printf("\n");
    printf("Available commands:\n");
    printf("  command &   Run command in background\n");
    printf("  jobs        Display job table\n");
    printf("  validate    Validate job records\n");
    printf("  cleanup     Remove completed jobs\n");
    printf("  help        Display help\n");
    printf("  exit        Exit program\n\n");
}


/* Main function */
int main(void)
{
    char line[MAX_LINE];

    printf("====================================================\n");
    printf("             SKILL-18 BACKGROUND JOB MANAGER\n");
    printf("====================================================\n");

    display_help();

    while (1) {

        /*
         * Check background processes before
         * displaying the prompt.
         */
        update_job_status();

        printf("skill18$ ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0)
            continue;

        /* Exit */
        if (strcmp(line, "exit") == 0) {

            terminate_all_jobs();

            printf("Skill-18 terminated.\n");

            break;
        }

        /* Jobs */
        if (strcmp(line, "jobs") == 0) {

            display_jobs();

            continue;
        }

        /* Validate */
        if (strcmp(line, "validate") == 0) {

            validate_jobs();

            continue;
        }

        /* Cleanup */
        if (strcmp(line, "cleanup") == 0) {

            cleanup_jobs();

            continue;
        }

        /* Help */
        if (strcmp(line, "help") == 0) {

            display_help();

            continue;
        }

        /*
         * Detect background job.
         */
        if (is_background_job(line)) {

            char original_command[MAX_LINE];

            strncpy(original_command,
                    line,
                    MAX_LINE - 1);

            original_command[MAX_LINE - 1] = '\0';

            remove_background_symbol(line);

            if (strlen(line) == 0) {

                printf("Error: Empty background command.\n");

                continue;
            }

            launch_background(line);
        }

        else {

            launch_foreground(line);
        }

        /*
         * Update finished jobs.
         */
        update_job_status();
    }

    return 0;
}
