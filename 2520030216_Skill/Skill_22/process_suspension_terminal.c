/*
 * Skill-22: Process Suspension and Terminal Control
 *
 * Objective 1:
 * Capture SIGTSTP, Suspend Processes, Update Job Table,
 * Preserve State, Resume Later, Test Suspension Workflow.
 *
 * Objective 2:
 * Create Process Groups, Assign Group IDs, Transfer Terminal Control,
 * Restore Ownership, Coordinate Signals, Validate Behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <termios.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 32


/* Job states */
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_COMPLETED,
    JOB_TERMINATED
} JobState;


/* Job information */
typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;

    char command[MAX_LINE];

    JobState state;

    int stop_signal;
    int exit_status;
} Job;


/* Job table */
Job job_table[MAX_JOBS];

int job_count = 0;
int next_job_id = 1;


/* Shell process group */
pid_t shell_pgid;

/* Terminal used by shell */
int shell_terminal;


/* Current foreground process group */
volatile sig_atomic_t foreground_pgid = 0;


/* SIGTSTP event flag */
volatile sig_atomic_t sigtstp_received = 0;


/* SIGCHLD event flag */
volatile sig_atomic_t sigchld_received = 0;


/*
 * Convert job state to text.
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

        case JOB_TERMINATED:
            return "TERMINATED";

        default:
            return "UNKNOWN";
    }
}


/*
 * Convert signal number to name.
 */
const char *signal_name(int sig)
{
    switch (sig) {

        case SIGTSTP:
            return "SIGTSTP";

        case SIGCONT:
            return "SIGCONT";

        case SIGINT:
            return "SIGINT";

        case SIGTERM:
            return "SIGTERM";

        case SIGCHLD:
            return "SIGCHLD";

        default:
            return "UNKNOWN";
    }
}


/*
 * SIGTSTP handler.
 *
 * The shell protects itself from Ctrl+Z.
 *
 * If a foreground process group is active,
 * SIGTSTP is forwarded to that group.
 */
void handle_sigtstp(int sig)
{
    sigtstp_received = 1;

    if (foreground_pgid > 0) {

        /*
         * Negative PGID sends SIGTSTP to
         * the entire foreground process group.
         */
        kill(-foreground_pgid, sig);
    }
}


/*
 * SIGCHLD handler.
 *
 * Only records the event.
 * waitpid() performs actual status processing.
 */
void handle_sigchld(int sig)
{
    (void)sig;

    sigchld_received = 1;
}


/*
 * Register signal handlers.
 */
void register_signal_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sigemptyset(&sa.sa_mask);

    /*
     * SIGTSTP handler.
     */
    sa.sa_handler = handle_sigtstp;

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {

        perror("sigaction SIGTSTP");

        exit(EXIT_FAILURE);
    }


    /*
     * SIGCHLD handler.
     */
    sa.sa_handler = handle_sigchld;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {

        perror("sigaction SIGCHLD");

        exit(EXIT_FAILURE);
    }
}


/*
 * Initialize the shell process group.
 */
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
     * Ignore terminal-control signals in shell.
     */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    /*
     * Protect shell from terminal stop.
     */
    signal(SIGTSTP, SIG_IGN);

    /*
     * Give terminal to shell.
     */
    if (tcsetpgrp(shell_terminal, shell_pgid) == -1) {

        perror("tcsetpgrp");
    }
}


/*
 * Parse command into arguments.
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
 * Check whether command ends with &.
 */
int is_background_command(char *command)
{
    int len = strlen(command);

    while (len > 0 &&
           (command[len - 1] == ' ' ||
            command[len - 1] == '\t')) {

        len--;
    }

    return len > 0 &&
           command[len - 1] == '&';
}


/*
 * Remove & from command.
 */
void remove_background_symbol(char *command)
{
    int len = strlen(command);

    while (len > 0 &&
           (command[len - 1] == ' ' ||
            command[len - 1] == '\t')) {

        command[--len] = '\0';
    }

    if (len > 0 &&
        command[len - 1] == '&') {

        command[--len] = '\0';

        while (len > 0 &&
               (command[len - 1] == ' ' ||
                command[len - 1] == '\t')) {

            command[--len] = '\0';
        }
    }
}


/*
 * Add a job to the job table.
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

    job_table[job_count].job_id =
        next_job_id++;

    job_table[job_count].pid = pid;

    job_table[job_count].pgid = pgid;

    strncpy(job_table[job_count].command,
            command,
            MAX_LINE - 1);

    job_table[job_count]
        .command[MAX_LINE - 1] = '\0';

    job_table[job_count].state = state;

    job_table[job_count].stop_signal = 0;

    job_table[job_count].exit_status = 0;

    return job_count++;
}


/*
 * Find a job using Job ID.
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
 * Remove a job from job table.
 */
void remove_job(int index)
{
    if (index < 0 ||
        index >= job_count)
        return;

    for (int i = index;
         i < job_count - 1;
         i++) {

        job_table[i] =
            job_table[i + 1];
    }

    job_count--;
}


/*
 * Update job states.
 */
void update_job_states(void)
{
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].state ==
                JOB_COMPLETED ||
            job_table[i].state ==
                JOB_TERMINATED) {

            continue;
        }

        int status;

        pid_t result =
            waitpid(
                job_table[i].pid,
                &status,
                WNOHANG | WUNTRACED | WCONTINUED
            );

        if (result == 0)
            continue;

        if (result == -1) {

            if (errno == ECHILD)
                continue;

            continue;
        }


        /*
         * Process has been stopped.
         */
        if (WIFSTOPPED(status)) {

            job_table[i].state =
                JOB_STOPPED;

            job_table[i].stop_signal =
                WSTOPSIG(status);

            printf("\n[%d] STOPPED by %s\n",
                   job_table[i].job_id,
                   signal_name(
                       WSTOPSIG(status)
                   ));
        }


        /*
         * Process has continued.
         */
        else if (WIFCONTINUED(status)) {

            job_table[i].state =
                JOB_RUNNING;

            printf("\n[%d] CONTINUED by SIGCONT\n",
                   job_table[i].job_id);
        }


        /*
         * Process exited normally.
         */
        else if (WIFEXITED(status)) {

            job_table[i].exit_status =
                WEXITSTATUS(status);

            job_table[i].state =
                JOB_COMPLETED;

            printf("\n[%d] COMPLETED\n",
                   job_table[i].job_id);
        }


        /*
         * Process terminated by signal.
         */
        else if (WIFSIGNALED(status)) {

            job_table[i].state =
                JOB_TERMINATED;

            job_table[i].stop_signal =
                WTERMSIG(status);

            printf("\n[%d] TERMINATED by %s\n",
                   job_table[i].job_id,
                   signal_name(
                       WTERMSIG(status)
                   ));
        }
    }
}


/*
 * Display job table.
 */
void list_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("============================================================\n");
    printf("                    SKILL-22 JOB TABLE\n");
    printf("============================================================\n");

    if (job_count == 0) {

        printf("No jobs available.\n");

        printf("============================================================\n");

        return;
    }

    printf("%-5s %-8s %-8s %-14s %-12s %s\n",
           "ID",
           "PID",
           "PGID",
           "STATE",
           "SIGNAL",
           "COMMAND");

    printf("------------------------------------------------------------\n");

    for (int i = 0;
         i < job_count;
         i++) {

        const char *sig = "-";

        if (job_table[i].stop_signal != 0) {

            sig =
                signal_name(
                    job_table[i].stop_signal
                );
        }

        printf("%-5d %-8d %-8d %-14s %-12s %s\n",
               job_table[i].job_id,
               job_table[i].pid,
               job_table[i].pgid,
               state_name(
                   job_table[i].state
               ),
               sig,
               job_table[i].command);
    }

    printf("============================================================\n");
}


/*
 * Give terminal control to a process group.
 */
void give_terminal_to(pid_t pgid)
{
    if (tcsetpgrp(
            shell_terminal,
            pgid) == -1) {

        perror("tcsetpgrp");
    }
}


/*
 * Restore terminal control to shell.
 */
void restore_terminal_to_shell(void)
{
    give_terminal_to(shell_pgid);
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

    int argc =
        parse_arguments(
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
         * Background process receives
         * normal terminal signal behavior.
         */
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }


    /*
     * Parent assigns PGID.
     */
    setpgid(pid, pid);


    /*
     * Add background job.
     */
    int index =
        add_job(
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
 * Launch foreground process.
 */
void launch_foreground(char *command)
{
    char command_copy[MAX_LINE];

    strncpy(command_copy,
            command,
            MAX_LINE - 1);

    command_copy[MAX_LINE - 1] = '\0';

    char *args[MAX_ARGS];

    int argc =
        parse_arguments(
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
         * Child creates its own process group.
         */
        if (setpgid(0, 0) == -1) {

            perror("setpgid");

            exit(EXIT_FAILURE);
        }

        /*
         * Foreground process uses default
         * signal behavior.
         */
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }


    /*
     * Parent places child into process group.
     */
    setpgid(pid, pid);

    /*
     * Store foreground process group.
     */
    foreground_pgid = pid;

    /*
     * Transfer terminal ownership.
     */
    give_terminal_to(pid);

    printf("Foreground process started.\n");
    printf("PID  = %d\n", pid);
    printf("PGID = %d\n", pid);

    printf("Press Ctrl+Z to send SIGTSTP.\n");
    printf("Press Ctrl+C to terminate the process.\n");


    /*
     * Wait for process completion or suspension.
     */
    while (1) {

        int status;

        pid_t result =
            waitpid(
                pid,
                &status,
                WUNTRACED
            );

        if (result == -1) {

            if (errno == EINTR)
                continue;

            perror("waitpid");

            break;
        }


        /*
         * Process stopped.
         */
        if (WIFSTOPPED(status)) {

            printf("\n");
            printf("Foreground process STOPPED.\n");
            printf("Stop signal: %s\n",
                   signal_name(
                       WSTOPSIG(status)
                   ));

            /*
             * Preserve process as a job.
             */
            int index =
                add_job(
                    pid,
                    pid,
                    command,
                    JOB_STOPPED
                );

            if (index >= 0) {

                job_table[index].stop_signal =
                    WSTOPSIG(status);
            }

            break;
        }


        /*
         * Process completed.
         */
        if (WIFEXITED(status)) {

            printf("\n");
            printf("Foreground process COMPLETED.\n");
            printf("Exit status: %d\n",
                   WEXITSTATUS(status));

            break;
        }


        /*
         * Process terminated by signal.
         */
        if (WIFSIGNALED(status)) {

            printf("\n");
            printf("Foreground process TERMINATED.\n");
            printf("Signal: %s\n",
                   signal_name(
                       WTERMSIG(status)
                   ));

            break;
        }
    }


    /*
     * Foreground process no longer owns terminal.
     */
    foreground_pgid = 0;

    /*
     * Restore terminal ownership to shell.
     */
    restore_terminal_to_shell();
}


/*
 * Resume a stopped job.
 */
void resume_job(int job_id)
{
    update_job_states();

    int index =
        find_job(job_id);

    if (index == -1) {

        printf("Job [%d] not found.\n",
               job_id);

        return;
    }

    Job *job =
        &job_table[index];


    if (job->state != JOB_STOPPED) {

        printf("Job [%d] is not stopped.\n",
               job_id);

        return;
    }


    printf("\n");
    printf("Resuming Job [%d]\n",
           job_id);

    printf("PID  = %d\n",
           job->pid);

    printf("PGID = %d\n",
           job->pgid);


    /*
     * Give terminal to process group.
     */
    give_terminal_to(job->pgid);


    /*
     * Send SIGCONT.
     */
    if (kill(
            -job->pgid,
            SIGCONT) == -1) {

        perror("SIGCONT");

        restore_terminal_to_shell();

        return;
    }


    job->state = JOB_RUNNING;

    printf("SIGCONT sent successfully.\n");
    printf("Process resumed.\n");


    /*
     * Wait for process to finish or stop again.
     */
    while (1) {

        int status;

        pid_t result =
            waitpid(
                job->pid,
                &status,
                WUNTRACED
            );

        if (result == -1) {

            if (errno == EINTR)
                continue;

            perror("waitpid");

            break;
        }


        if (WIFSTOPPED(status)) {

            job->state =
                JOB_STOPPED;

            job->stop_signal =
                WSTOPSIG(status);

            printf("\nJob [%d] stopped again.\n",
                   job->job_id);

            break;
        }


        if (WIFEXITED(status)) {

            job->state =
                JOB_COMPLETED;

            job->exit_status =
                WEXITSTATUS(status);

            printf("\nJob [%d] completed.\n",
                   job->job_id);

            break;
        }


        if (WIFSIGNALED(status)) {

            job->state =
                JOB_TERMINATED;

            printf("\nJob [%d] terminated.\n",
                   job->job_id);

            break;
        }
    }


    /*
     * Restore shell terminal ownership.
     */
    restore_terminal_to_shell();

    /*
     * Remove finished job.
     */
    if (job->state == JOB_COMPLETED ||
        job->state == JOB_TERMINATED) {

        remove_job(index);
    }
}


/*
 * Validate job table.
 */
void validate_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("============================================================\n");
    printf("                    JOB VALIDATION\n");
    printf("============================================================\n");

    if (job_count == 0) {

        printf("Job table is empty.\n");
        printf("Validation: OK\n");

        printf("============================================================\n");

        return;
    }

    int valid = 1;

    for (int i = 0;
         i < job_count;
         i++) {

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
               state_name(
                   job_table[i].state
               ));
    }

    if (valid)
        printf("Validation: ALL JOB RECORDS VALID\n");
    else
        printf("Validation: INVALID RECORD FOUND\n");

    printf("============================================================\n");
}


/*
 * Display process group information.
 */
void display_process_groups(void)
{
    printf("\n");
    printf("========== PROCESS GROUP INFORMATION ==========\n");

    printf("Shell PID  : %d\n",
           getpid());

    printf("Shell PGID : %d\n",
           shell_pgid);

    printf("Current PGID: %d\n",
           getpgrp());

    printf("Foreground PGID: %d\n",
           foreground_pgid);

    printf("===============================================\n");
}


/*
 * Display signal information.
 */
void display_signals(void)
{
    printf("\n");
    printf("========== TERMINAL SIGNALS ==========\n");

    printf("SIGTSTP (%d) -> Stop process\n",
           SIGTSTP);

    printf("SIGCONT (%d) -> Continue process\n",
           SIGCONT);

    printf("SIGINT  (%d) -> Interrupt process\n",
           SIGINT);

    printf("SIGTERM (%d) -> Terminate process\n",
           SIGTERM);

    printf("SIGCHLD (%d) -> Child state changed\n",
           SIGCHLD);

    printf("======================================\n");
}


/*
 * Display help.
 */
void show_help(void)
{
    printf("\n");
    printf("Available commands:\n");

    printf("  command &       Start background job\n");
    printf("  jobs             Display job table\n");
    printf("  resume <id>      Resume stopped job\n");
    printf("  validate         Validate job records\n");
    printf("  groups           Display process groups\n");
    printf("  signals          Display signal information\n");
    printf("  help             Display help\n");
    printf("  cleanup          Remove completed jobs\n");
    printf("  exit             Exit program\n");

    printf("\n");
}


/*
 * Remove completed jobs.
 */
void cleanup_jobs(void)
{
    update_job_states();

    int removed = 0;

    for (int i = job_count - 1;
         i >= 0;
         i--) {

        if (job_table[i].state ==
                JOB_COMPLETED ||
            job_table[i].state ==
                JOB_TERMINATED) {

            remove_job(i);

            removed++;
        }
    }

    printf("Cleanup: %d job(s) removed.\n",
           removed);
}


/*
 * Main function.
 */
int main(void)
{
    char line[MAX_LINE];


    /*
     * Initialize shell process group.
     */
    initialize_shell();


    /*
     * Register signal handlers.
     */
    register_signal_handlers();


    printf("============================================================\n");
    printf("       SKILL-22 PROCESS SUSPENSION & TERMINAL CONTROL\n");
    printf("============================================================\n");

    show_help();


    while (1) {

        /*
         * Update job states.
         */
        update_job_states();


        printf("skill22$ ");

        fflush(stdout);


        /*
         * Read command.
         */
        if (fgets(
                line,
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
        if (strcmp(line, "exit") == 0)
            break;


        /*
         * List jobs.
         */
        if (strcmp(line, "jobs") == 0) {

            list_jobs();

            continue;
        }


        /*
         * Resume job.
         */
        if (strncmp(line, "resume ", 7) == 0) {

            int job_id;

            if (sscanf(
                    line + 7,
                    "%d",
                    &job_id) != 1) {

                printf("Usage: resume <job_id>\n");

                continue;
            }

            resume_job(job_id);

            continue;
        }


        /*
         * Validate.
         */
        if (strcmp(line, "validate") == 0) {

            validate_jobs();

            continue;
        }


        /*
         * Process group information.
         */
        if (strcmp(line, "groups") == 0) {

            display_process_groups();

            continue;
        }


        /*
         * Signal information.
         */
        if (strcmp(line, "signals") == 0) {

            display_signals();

            continue;
        }


        /*
         * Cleanup.
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
         * Background command.
         */
        if (is_background_command(line)) {

            remove_background_symbol(line);

            if (strlen(line) == 0) {

                printf("Error: Empty command.\n");

                continue;
            }

            launch_background(line);

            continue;
        }


        /*
         * Otherwise run foreground command.
         */
        launch_foreground(line);
    }


    /*
     * Restore terminal to shell.
     */
    restore_terminal_to_shell();


    /*
     * Terminate remaining jobs.
     */
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].state ==
                JOB_RUNNING ||
            job_table[i].state ==
                JOB_STOPPED) {

            kill(
                -job_table[i].pgid,
                SIGTERM
            );
        }
    }


    /*
     * Reap children.
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


    printf("\nSkill-22 terminated.\n");

    return 0;
}
