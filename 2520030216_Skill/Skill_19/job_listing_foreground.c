
/*
 * Skill-19: Job Listing and Foreground Switching
 *
 * Objective 1:
 * List Active Jobs, Display Status Information, Retrieve Job IDs,
 * Format Output, Update State Information, Test Job Listings.
 *
 * Objective 2:
 * Identify Target Job, Transfer Terminal Control, Resume Processes,
 * Wait for Completion, Update Job State, Test Foreground Switching.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 32

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_COMPLETED,
    JOB_FAILED
} JobState;

typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;

    char command[MAX_LINE];

    JobState state;

    int status;
} Job;

/* Job table */
Job job_table[MAX_JOBS];

int job_count = 0;
int next_job_id = 1;

/* Shell information */
pid_t shell_pgid;
int shell_terminal;
struct termios shell_tmodes;


/* Convert state to readable text */
const char *state_name(JobState state)
{
    switch (state) {

        case JOB_RUNNING:
            return "Running";

        case JOB_STOPPED:
            return "Stopped";

        case JOB_COMPLETED:
            return "Completed";

        case JOB_FAILED:
            return "Failed";

        default:
            return "Unknown";
    }
}


/* Initialize shell process group */
void initialize_shell(void)
{
    shell_terminal = STDIN_FILENO;

    shell_pgid = getpid();

    /*
     * Put shell into its own process group.
     */
    if (setpgid(shell_pgid, shell_pgid) == -1) {

        if (errno != EPERM) {
            perror("setpgid");
        }
    }

    /*
     * Give terminal control to the shell.
     */
    if (tcsetpgrp(shell_terminal, shell_pgid) == -1) {
        perror("tcsetpgrp");
    }

    /*
     * Save terminal settings.
     */
    tcgetattr(shell_terminal, &shell_tmodes);
}


/* Parse command arguments */
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


/* Detect '&' at end of command */
int is_background_command(char *command)
{
    int len = strlen(command);

    while (len > 0 &&
           (command[len - 1] == ' ' ||
            command[len - 1] == '\t' ||
            command[len - 1] == '\n')) {

        len--;
    }

    return (len > 0 && command[len - 1] == '&');
}


/* Remove '&' from command */
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


/* Add a job */
int add_job(pid_t pid, pid_t pgid,
            const char *command,
            JobState state)
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

    job_table[job_count].state = state;

    job_table[job_count].status = 0;

    return job_count++;
}


/* Find job using job ID */
int find_job(int job_id)
{
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].job_id == job_id)
            return i;
    }

    return -1;
}


/* Remove job from table */
void remove_job(int index)
{
    if (index < 0 || index >= job_count)
        return;

    for (int i = index; i < job_count - 1; i++) {

        job_table[i] = job_table[i + 1];
    }

    job_count--;
}


/*
 * Update the state of every job.
 *
 * WNOHANG:
 * Do not block while checking running jobs.
 *
 * WUNTRACED:
 * Detect stopped processes.
 */
void update_job_states(void)
{
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].state == JOB_COMPLETED ||
            job_table[i].state == JOB_FAILED) {

            continue;
        }

        int status;

        pid_t result = waitpid(
            job_table[i].pid,
            &status,
            WNOHANG | WUNTRACED
        );

        if (result == 0) {

            /*
             * Process is still running.
             */
            continue;
        }

        if (result == -1) {

            if (errno == ECHILD)
                continue;

            continue;
        }

        if (WIFSTOPPED(status)) {

            job_table[i].state = JOB_STOPPED;

            job_table[i].status =
                WSTOPSIG(status);

            printf("\n[%d] Stopped %s\n",
                   job_table[i].job_id,
                   job_table[i].command);
        }

        else if (WIFEXITED(status)) {

            job_table[i].status =
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

            job_table[i].status =
                WTERMSIG(status);

            job_table[i].state =
                JOB_FAILED;
        }
    }
}


/* Display active jobs */
void list_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("===============================================================\n");
    printf("                         ACTIVE JOBS\n");
    printf("===============================================================\n");

    if (job_count == 0) {

        printf("No jobs available.\n");

        printf("===============================================================\n");

        return;
    }

    printf("%-5s %-8s %-8s %-12s %s\n",
           "ID",
           "PID",
           "PGID",
           "STATE",
           "COMMAND");

    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < job_count; i++) {

        printf("%-5d %-8d %-8d %-12s %s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               state_name(job_table[i].state),
               job_table[i].command);
    }

    printf("===============================================================\n");
}


/* Retrieve and display a specific job */
void show_job_id(int job_id)
{
    update_job_states();

    int index = find_job(job_id);

    if (index == -1) {

        printf("Job [%d] not found.\n", job_id);

        return;
    }

    printf("\nJob Information\n");

    printf("-----------------------------\n");

    printf("Job ID : %d\n",
           job_table[index].job_id);

    printf("PID    : %d\n",
           job_table[index].pid);

    printf("PGID   : %d\n",
           job_table[index].pgid);

    printf("State  : %s\n",
           state_name(job_table[index].state));

    printf("Command: %s\n",
           job_table[index].command);

    printf("-----------------------------\n");
}


/*
 * Transfer terminal control to a process group.
 */
void give_terminal_to(pid_t pgid)
{
    if (tcsetpgrp(shell_terminal, pgid) == -1) {

        perror("tcsetpgrp");
    }
}


/*
 * Restore terminal control to shell.
 */
void give_terminal_to_shell(void)
{
    give_terminal_to(shell_pgid);

    /*
     * Restore terminal settings.
     */
    tcgetattr(shell_terminal, &shell_tmodes);
}


/*
 * Bring a job to foreground.
 */
void foreground_job(int job_id)
{
    update_job_states();

    int index = find_job(job_id);

    if (index == -1) {

        printf("fg: Job [%d] not found.\n",
               job_id);

        return;
    }

    Job *job = &job_table[index];

    printf("\n");
    printf("Bringing Job [%d] to foreground...\n",
           job->job_id);

    printf("Command: %s\n",
           job->command);

    /*
     * Transfer terminal control to job process group.
     */
    give_terminal_to(job->pgid);

    /*
     * If job was stopped, resume it.
     */
    if (job->state == JOB_STOPPED) {

        printf("Resuming process group %d...\n",
               job->pgid);

        if (kill(-job->pgid, SIGCONT) == -1) {

            perror("SIGCONT");

            give_terminal_to_shell();

            return;
        }
    }

    job->state = JOB_RUNNING;

    /*
     * Wait until the foreground job:
     *
     * 1. Completes
     * 2. Terminates
     * 3. Becomes stopped
     */
    int status;

    pid_t result;

    do {

        result = waitpid(
            job->pid,
            &status,
            WUNTRACED
        );

    } while (result == -1 && errno == EINTR);

    /*
     * Restore terminal control to shell.
     */
    give_terminal_to_shell();

    /*
     * Update job state.
     */
    if (result == job->pid) {

        if (WIFSTOPPED(status)) {

            job->state = JOB_STOPPED;

            job->status =
                WSTOPSIG(status);

            printf("\nJob [%d] stopped.\n",
                   job->job_id);
        }

        else if (WIFEXITED(status)) {

            job->status =
                WEXITSTATUS(status);

            if (WEXITSTATUS(status) == 0) {

                job->state =
                    JOB_COMPLETED;

                printf("\nJob [%d] completed.\n",
                       job->job_id);
            }

            else {

                job->state =
                    JOB_FAILED;

                printf("\nJob [%d] failed.\n",
                       job->job_id);
            }
        }

        else if (WIFSIGNALED(status)) {

            job->state = JOB_FAILED;

            job->status =
                WTERMSIG(status);

            printf("\nJob [%d] terminated by signal.\n",
                   job->job_id);
        }
    }

    /*
     * Remove completed/failed jobs.
     */
    if (job->state == JOB_COMPLETED ||
        job->state == JOB_FAILED) {

        remove_job(index);
    }
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

    int argc = parse_arguments(
        command_copy,
        args
    );

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
         * Execute command.
         */
        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent sets child's process group.
     */
    setpgid(pid, pid);

    /*
     * Add job to job table.
     */
    int index = add_job(
        pid,
        pid,
        command,
        JOB_RUNNING
    );

    if (index >= 0) {

        printf("[%d] %d started in background\n",
               job_table[index].job_id,
               pid);
    }
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

    int argc = parse_arguments(
        command_copy,
        args
    );

    if (argc <= 0)
        return;

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");

        return;
    }

    if (pid == 0) {

        /*
         * Child gets its own process group.
         */
        setpgid(0, 0);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent sets process group.
     */
    setpgid(pid, pid);

    /*
     * Give terminal to foreground process.
     */
    give_terminal_to(pid);

    int status;

    waitpid(
        pid,
        &status,
        WUNTRACED
    );

    /*
     * Return terminal control to shell.
     */
    give_terminal_to_shell();

    if (WIFSTOPPED(status)) {

        /*
         * Store stopped foreground command
         * as a job.
         */
        add_job(
            pid,
            pid,
            command,
            JOB_STOPPED
        );

        printf("\nForeground process stopped and added to jobs.\n");
    }
}


/* Cleanup completed jobs */
void cleanup_jobs(void)
{
    update_job_states();

    int removed = 0;

    for (int i = job_count - 1; i >= 0; i--) {

        if (job_table[i].state == JOB_COMPLETED ||
            job_table[i].state == JOB_FAILED) {

            remove_job(i);

            removed++;
        }
    }

    printf("Cleanup: %d completed/failed job(s) removed.\n",
           removed);
}


/* Validate job records */
void validate_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("===============================================================\n");
    printf("                       JOB VALIDATION\n");
    printf("===============================================================\n");

    if (job_count == 0) {

        printf("Job table is empty.\n");
        printf("Validation: OK\n");

        printf("===============================================================\n");

        return;
    }

    int valid = 1;

    for (int i = 0; i < job_count; i++) {

        if (job_table[i].job_id <= 0 ||
            job_table[i].pid <= 0 ||
            job_table[i].pgid <= 0 ||
            strlen(job_table[i].command) == 0) {

            valid = 0;
        }

        printf("Job [%d] -> PID=%d PGID=%d STATE=%s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               state_name(job_table[i].state));
    }

    if (valid)
        printf("Validation: ALL JOB RECORDS VALID\n");
    else
        printf("Validation: INVALID RECORD FOUND\n");

    printf("===============================================================\n");
}


/* Print help */
void show_help(void)
{
    printf("\n");
    printf("Commands:\n");
    printf("  command &       Start a background job\n");
    printf("  jobs             List active jobs\n");
    printf("  fg <job_id>      Bring job to foreground\n");
    printf("  info <job_id>    Display specific job information\n");
    printf("  validate         Validate job records\n");
    printf("  cleanup          Remove completed jobs\n");
    printf("  help             Display this help\n");
    printf("  exit             Exit program\n");
    printf("\n");
}


/* Main function */
int main(void)
{
    char line[MAX_LINE];

    /*
     * Initialize shell terminal.
     */
    initialize_shell();

    printf("===============================================================\n");
    printf("       SKILL-19 JOB LISTING & FOREGROUND SWITCHING\n");
    printf("===============================================================\n");

    show_help();

    while (1) {

        /*
         * Update job information before prompt.
         */
        update_job_states();

        printf("skill19$ ");

        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0)
            continue;

        /*
         * Exit.
         */
        if (strcmp(line, "exit") == 0) {

            break;
        }

        /*
         * jobs command.
         */
        if (strcmp(line, "jobs") == 0) {

            list_jobs();

            continue;
        }

        /*
         * validate command.
         */
        if (strcmp(line, "validate") == 0) {

            validate_jobs();

            continue;
        }

        /*
         * cleanup command.
         */
        if (strcmp(line, "cleanup") == 0) {

            cleanup_jobs();

            continue;
        }

        /*
         * help command.
         */
        if (strcmp(line, "help") == 0) {

            show_help();

            continue;
        }

        /*
         * fg command.
         */
        if (strncmp(line, "fg ", 3) == 0) {

            int job_id;

            if (sscanf(line + 3, "%d", &job_id) != 1) {

                printf("Usage: fg <job_id>\n");

                continue;
            }

            foreground_job(job_id);

            continue;
        }

        /*
         * info command.
         */
        if (strncmp(line, "info ", 5) == 0) {

            int job_id;

            if (sscanf(line + 5, "%d", &job_id) != 1) {

                printf("Usage: info <job_id>\n");

                continue;
            }

            show_job_id(job_id);

            continue;
        }

        /*
         * Check for background command.
         */
        if (is_background_command(line)) {

            remove_background_symbol(line);

            if (strlen(line) == 0) {

                printf("Error: Empty background command.\n");

                continue;
            }

            launch_background(line);

            continue;
        }

        /*
         * Otherwise execute in foreground.
         */
        launch_foreground(line);
    }

    /*
     * Return terminal control to shell.
     */
    give_terminal_to_shell();

    /*
     * Terminate any remaining background jobs.
     */
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].state == JOB_RUNNING ||
            job_table[i].state == JOB_STOPPED) {

            kill(-job_table[i].pgid, SIGTERM);
        }
    }

    /*
     * Reap remaining children.
     */
    for (int i = 0; i < job_count; i++) {

        waitpid(
            job_table[i].pid,
            NULL,
            0
        );
    }

    printf("\nSkill-19 terminated.\n");

    return 0;
}
