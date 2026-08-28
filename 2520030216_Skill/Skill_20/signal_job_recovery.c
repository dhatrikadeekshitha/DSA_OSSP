/*
 * Skill-20: Signal Handling and Stopped Job Recovery
 *
 * Objective 1:
 * Analyze Signal Delivery, Understand Signal Masks, Review Process Groups,
 * Explore Terminal Signals, Test Signal Behavior.
 *
 * Objective 2:
 * Resume Stopped Jobs, Send Continue Signals, Update Job Status,
 * Maintain Job Table, Verify Execution, Test Background Recovery.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

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
    int last_signal;
    int exit_status;
} Job;

Job job_table[MAX_JOBS];

int job_count = 0;
int next_job_id = 1;


/*
 * Convert signal number into readable name.
 */
const char *signal_name(int sig)
{
    switch (sig) {

        case SIGINT:
            return "SIGINT";

        case SIGSTOP:
            return "SIGSTOP";

        case SIGCONT:
            return "SIGCONT";

        case SIGTERM:
            return "SIGTERM";

        case SIGTSTP:
            return "SIGTSTP";

        default:
            return "UNKNOWN";
    }
}


/*
 * Convert job state into readable text.
 */
const char *state_name(JobState state)
{
    switch (state) {

        case JOB_RUNNING:
            return "RUNNING";

        case JOB_STOPPED:
            return "STOPPED";

        case JOB_COMPLETED:
            return "COMPLETED";

        case JOB_FAILED:
            return "FAILED";

        default:
            return "UNKNOWN";
    }
}


/*
 * Parse command arguments.
 */
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


/*
 * Check whether command ends with '&'.
 */
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


/*
 * Remove '&' from command.
 */
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


/*
 * Add job to job table.
 */
int add_job(pid_t pid,
            pid_t pgid,
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

    job_table[job_count].last_signal = 0;

    job_table[job_count].exit_status = 0;

    return job_count++;
}


/*
 * Find job by job ID.
 */
int find_job(int job_id)
{
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].job_id == job_id)
            return i;
    }

    return -1;
}


/*
 * Remove job from table.
 */
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
 * Display signal mask information.
 */
void display_signal_mask(void)
{
    sigset_t mask;

    if (sigprocmask(SIG_BLOCK, NULL, &mask) == -1) {

        perror("sigprocmask");

        return;
    }

    printf("\n");
    printf("========== SIGNAL MASK ==========\n");

    printf("SIGINT  : %s\n",
           sigismember(&mask, SIGINT) ? "BLOCKED" : "UNBLOCKED");

    printf("SIGTERM : %s\n",
           sigismember(&mask, SIGTERM) ? "BLOCKED" : "UNBLOCKED");

    printf("SIGTSTP : %s\n",
           sigismember(&mask, SIGTSTP) ? "BLOCKED" : "UNBLOCKED");

    printf("SIGCONT : %s\n",
           sigismember(&mask, SIGCONT) ? "BLOCKED" : "UNBLOCKED");

    printf("SIGCHLD : %s\n",
           sigismember(&mask, SIGCHLD) ? "BLOCKED" : "UNBLOCKED");

    printf("=================================\n");
}


/*
 * Display information about signals.
 */
void display_signal_info(void)
{
    printf("\n");
    printf("========== SIGNAL INFORMATION ==========\n");

    printf("SIGINT  (%d)  : Interrupt process\n", SIGINT);
    printf("SIGSTOP (%d)  : Stop process immediately\n", SIGSTOP);
    printf("SIGCONT (%d)  : Continue stopped process\n", SIGCONT);
    printf("SIGTERM (%d)  : Request process termination\n", SIGTERM);
    printf("SIGTSTP (%d)  : Terminal stop signal\n", SIGTSTP);
    printf("SIGCHLD (%d)  : Child process state changed\n", SIGCHLD);

    printf("=========================================\n");
}


/*
 * Update state of all jobs.
 *
 * WNOHANG  -> non-blocking status check
 * WUNTRACED -> detect stopped processes
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

            job_table[i].last_signal =
                WSTOPSIG(status);

            printf("\n[%d] STOPPED by %s\n",
                   job_table[i].job_id,
                   signal_name(WSTOPSIG(status)));
        }

        else if (WIFCONTINUED(status)) {

            job_table[i].state = JOB_RUNNING;

            job_table[i].last_signal = SIGCONT;

            printf("\n[%d] CONTINUED by SIGCONT\n",
                   job_table[i].job_id);
        }

        else if (WIFEXITED(status)) {

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

            job_table[i].last_signal =
                WTERMSIG(status);

            job_table[i].state =
                JOB_FAILED;
        }
    }
}


/*
 * List all jobs.
 */
void list_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("============================================================\n");
    printf("                     JOB TABLE\n");
    printf("============================================================\n");

    if (job_count == 0) {

        printf("No jobs available.\n");

        printf("============================================================\n");

        return;
    }

    printf("%-5s %-8s %-8s %-12s %-12s %s\n",
           "ID",
           "PID",
           "PGID",
           "STATE",
           "SIGNAL",
           "COMMAND");

    printf("------------------------------------------------------------\n");

    for (int i = 0; i < job_count; i++) {

        const char *sig =
            job_table[i].last_signal ?
            signal_name(job_table[i].last_signal) :
            "-";

        printf("%-5d %-8d %-8d %-12s %-12s %s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               state_name(job_table[i].state),
               sig,
               job_table[i].command);
    }

    printf("============================================================\n");
}


/*
 * Stop a running job using SIGSTOP.
 */
void stop_job(int job_id)
{
    update_job_states();

    int index = find_job(job_id);

    if (index == -1) {

        printf("Job [%d] not found.\n", job_id);

        return;
    }

    Job *job = &job_table[index];

    if (job->state != JOB_RUNNING) {

        printf("Job [%d] is not running.\n",
               job_id);

        return;
    }

    printf("Sending SIGSTOP to Job [%d]...\n",
           job_id);

    /*
     * Negative PGID sends signal to the
     * complete process group.
     */
    if (kill(-job->pgid, SIGSTOP) == -1) {

        perror("SIGSTOP");

        return;
    }

    job->last_signal = SIGSTOP;

    job->state = JOB_STOPPED;

    printf("Job [%d] is now STOPPED.\n",
           job_id);
}


/*
 * Resume stopped job using SIGCONT.
 */
void resume_job(int job_id)
{
    update_job_states();

    int index = find_job(job_id);

    if (index == -1) {

        printf("Job [%d] not found.\n",
               job_id);

        return;
    }

    Job *job = &job_table[index];

    if (job->state != JOB_STOPPED) {

        printf("Job [%d] is not stopped.\n",
               job_id);

        return;
    }

    printf("Sending SIGCONT to Job [%d]...\n",
           job_id);

    /*
     * Resume complete process group.
     */
    if (kill(-job->pgid, SIGCONT) == -1) {

        perror("SIGCONT");

        return;
    }

    job->last_signal = SIGCONT;

    job->state = JOB_RUNNING;

    printf("Job [%d] resumed successfully.\n",
           job_id);
}


/*
 * Display detailed job information.
 */
void show_job_info(int job_id)
{
    update_job_states();

    int index = find_job(job_id);

    if (index == -1) {

        printf("Job [%d] not found.\n",
               job_id);

        return;
    }

    Job *job = &job_table[index];

    printf("\n");
    printf("========== JOB INFORMATION ==========\n");

    printf("Job ID       : %d\n",
           job->job_id);

    printf("PID          : %d\n",
           job->pid);

    printf("Process Group: %d\n",
           job->pgid);

    printf("State        : %s\n",
           state_name(job->state));

    printf("Command      : %s\n",
           job->command);

    if (job->last_signal)
        printf("Last Signal  : %s\n",
               signal_name(job->last_signal));
    else
        printf("Last Signal  : None\n");

    printf("Exit Status  : %d\n",
           job->exit_status);

    printf("=====================================\n");
}


/*
 * Launch background process.
 */
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
         * Create a separate process group.
         */
        if (setpgid(0, 0) == -1) {

            perror("setpgid");

            exit(EXIT_FAILURE);
        }

        /*
         * Child starts with default signal handling.
         */
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent assigns child to its own process group.
     */
    setpgid(pid, pid);

    /*
     * Store process and group information.
     */
    int index = add_job(
        pid,
        pid,
        command,
        JOB_RUNNING
    );

    if (index >= 0) {

        printf("[%d] PID=%d PGID=%d RUNNING\n",
               job_table[index].job_id,
               pid,
               pid);
    }
}


/*
 * Launch foreground command.
 */
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

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    int status;

    waitpid(pid, &status, 0);
}


/*
 * Validate job records.
 */
void validate_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("========== JOB VALIDATION ==========\n");

    if (job_count == 0) {

        printf("Job table is empty.\n");
        printf("Validation: OK\n");

        printf("=====================================\n");

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

        printf("Job [%d] : PID=%d PGID=%d STATE=%s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               state_name(job_table[i].state));
    }

    if (valid)
        printf("Validation: ALL JOB RECORDS VALID\n");
    else
        printf("Validation: INVALID RECORD FOUND\n");

    printf("=====================================\n");
}


/*
 * Remove completed jobs.
 */
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

    printf("Cleanup: %d job(s) removed.\n",
           removed);
}


/*
 * Test signal delivery.
 */
void test_signal_delivery(void)
{
    printf("\n");
    printf("========== SIGNAL DELIVERY TEST ==========\n");

    printf("Current Process PID : %d\n",
           getpid());

    printf("Current Process PGID: %d\n",
           getpgrp());

    printf("\nSignal delivery examples:\n");

    printf("SIGINT  -> interrupt process\n");
    printf("SIGSTOP -> stop process\n");
    printf("SIGCONT -> resume process\n");
    printf("SIGTERM -> terminate process\n");

    printf("\nSignal delivery test completed.\n");

    printf("==========================================\n");
}


/*
 * Display help.
 */
void show_help(void)
{
    printf("\n");
    printf("Available commands:\n");
    printf("  command &       Start a background job\n");
    printf("  jobs             List jobs and states\n");
    printf("  stop <id>        Stop a running job using SIGSTOP\n");
    printf("  resume <id>      Resume a stopped job using SIGCONT\n");
    printf("  info <id>        Display job information\n");
    printf("  signals          Display signal information\n");
    printf("  mask             Display signal mask\n");
    printf("  test             Test signal delivery information\n");
    printf("  validate         Validate job records\n");
    printf("  cleanup          Remove completed jobs\n");
    printf("  help             Display help\n");
    printf("  exit             Exit program\n");
    printf("\n");
}


/*
 * Main function.
 */
int main(void)
{
    char line[MAX_LINE];

    printf("============================================================\n");
    printf("          SKILL-20 SIGNAL & JOB RECOVERY MANAGER\n");
    printf("============================================================\n");

    show_help();

    while (1) {

        /*
         * Update background jobs before prompt.
         */
        update_job_states();

        printf("skill20$ ");

        fflush(stdout);

        if (fgets(line,
                  sizeof(line),
                  stdin) == NULL) {

            break;
        }

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
         * List jobs.
         */
        if (strcmp(line, "jobs") == 0) {

            list_jobs();

            continue;
        }


        /*
         * Display signals.
         */
        if (strcmp(line, "signals") == 0) {

            display_signal_info();

            continue;
        }


        /*
         * Display signal mask.
         */
        if (strcmp(line, "mask") == 0) {

            display_signal_mask();

            continue;
        }


        /*
         * Signal delivery test.
         */
        if (strcmp(line, "test") == 0) {

            test_signal_delivery();

            continue;
        }


        /*
         * Validate job table.
         */
        if (strcmp(line, "validate") == 0) {

            validate_jobs();

            continue;
        }


        /*
         * Cleanup jobs.
         */
        if (strcmp(line, "cleanup") == 0) {

            cleanup_jobs();

            continue;
        }


        /*
         * Help.
         */
        if (strcmp(line, "help") == 0) {

            show_help();

            continue;
        }


        /*
         * Stop command.
         */
        if (strncmp(line, "stop ", 5) == 0) {

            int job_id;

            if (sscanf(line + 5,
                       "%d",
                       &job_id) != 1) {

                printf("Usage: stop <job_id>\n");

                continue;
            }

            stop_job(job_id);

            continue;
        }


        /*
         * Resume command.
         */
        if (strncmp(line, "resume ", 7) == 0) {

            int job_id;

            if (sscanf(line + 7,
                       "%d",
                       &job_id) != 1) {

                printf("Usage: resume <job_id>\n");

                continue;
            }

            resume_job(job_id);

            continue;
        }


        /*
         * Information command.
         */
        if (strncmp(line, "info ", 5) == 0) {

            int job_id;

            if (sscanf(line + 5,
                       "%d",
                       &job_id) != 1) {

                printf("Usage: info <job_id>\n");

                continue;
            }

            show_job_info(job_id);

            continue;
        }


        /*
         * Detect background command.
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
         * Otherwise execute as foreground command.
         */
        launch_foreground(line);
    }


    /*
     * Terminate remaining jobs before exit.
     */
    for (int i = 0; i < job_count; i++) {

        if (job_table[i].state == JOB_RUNNING ||
            job_table[i].state == JOB_STOPPED) {

            kill(-job_table[i].pgid,
                 SIGTERM);
        }
    }


    /*
     * Reap child processes.
     */
    for (int i = 0; i < job_count; i++) {

        waitpid(job_table[i].pid,
                NULL,
                0);
    }


    printf("\nSkill-20 terminated.\n");

    return 0;
}
