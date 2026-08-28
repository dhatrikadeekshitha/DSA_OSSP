/*
 * Skill-21: Signal Interrupt Handling
 *
 * Objective 1:
 * Register Handlers, Configure Signal Actions, Handle Interrupts,
 * Manage Signal Context, Verify Stability, Test Signal Events.
 *
 * Objective 2:
 * Capture SIGINT, Forward Signals, Protect Shell Process,
 * Terminate Foreground Jobs, Update Job States,
 * Test Interrupt Handling.
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
    JOB_TERMINATED
} JobState;

typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;
    char command[MAX_LINE];
    JobState state;
    int last_signal;
} Job;


/* Job table */
Job job_table[MAX_JOBS];

int job_count = 0;
int next_job_id = 1;


/*
 * Shell process group.
 */
pid_t shell_pgid;

/*
 * PID of current foreground process.
 */
volatile sig_atomic_t foreground_pid = 0;

/*
 * Flag used by SIGINT handler.
 */
volatile sig_atomic_t sigint_received = 0;

/*
 * Flag used by SIGCHLD handler.
 */
volatile sig_atomic_t child_changed = 0;


/*
 * Convert job state to string.
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

        case SIGINT:
            return "SIGINT";

        case SIGTERM:
            return "SIGTERM";

        case SIGTSTP:
            return "SIGTSTP";

        case SIGCHLD:
            return "SIGCHLD";

        default:
            return "UNKNOWN";
    }
}


/*
 * SIGINT handler.
 *
 * The shell itself does NOT terminate.
 * Instead, SIGINT is forwarded to the
 * foreground process group.
 */
void handle_sigint(int sig)
{
    sigint_received = 1;

    /*
     * Only forward SIGINT if a foreground
     * process currently exists.
     */
    if (foreground_pid > 0) {

        /*
         * Negative PID targets the complete
         * process group.
         */
        kill(-foreground_pid, sig);
    }
}


/*
 * SIGCHLD handler.
 *
 * This only records that a child changed state.
 * Actual waitpid() processing is done safely
 * in the main execution flow.
 */
void handle_sigchld(int sig)
{
    (void)sig;

    child_changed = 1;
}


/*
 * SIGTSTP handler.
 *
 * Protect the shell from Ctrl+Z.
 */
void handle_sigtstp(int sig)
{
    (void)sig;

    /*
     * Shell ignores terminal stop request.
     */
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
     * SIGINT handler.
     */
    sa.sa_handler = handle_sigint;

    if (sigaction(SIGINT, &sa, NULL) == -1) {

        perror("sigaction SIGINT");

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


    /*
     * SIGTSTP handler.
     */
    sa.sa_handler = handle_sigtstp;

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {

        perror("sigaction SIGTSTP");

        exit(EXIT_FAILURE);
    }
}


/*
 * Initialize shell process group.
 */
void initialize_shell(void)
{
    shell_pgid = getpid();

    /*
     * Place shell into its own process group.
     */
    if (setpgid(shell_pgid, shell_pgid) == -1) {

        if (errno != EPERM) {
            perror("setpgid");
        }
    }

    /*
     * Shell ignores interactive terminal signals.
     */
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
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
            command[len - 1] == '\t')) {

        len--;
    }

    return (len > 0 &&
            command[len - 1] == '&');
}


/*
 * Remove '&' from command.
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

    job_table[job_count].last_signal = 0;

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
 * Remove job from table.
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
                WNOHANG | WUNTRACED
            );

        if (result == 0)
            continue;

        if (result == -1)
            continue;

        if (WIFSTOPPED(status)) {

            job_table[i].state =
                JOB_STOPPED;

            job_table[i].last_signal =
                WSTOPSIG(status);
        }

        else if (WIFEXITED(status)) {

            job_table[i].state =
                JOB_COMPLETED;

            job_table[i].last_signal = 0;
        }

        else if (WIFSIGNALED(status)) {

            job_table[i].state =
                JOB_TERMINATED;

            job_table[i].last_signal =
                WTERMSIG(status);
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
    printf("                     JOB TABLE\n");
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

        if (job_table[i].last_signal != 0) {

            sig =
                signal_name(
                    job_table[i].last_signal
                );
        }

        printf("%-5d %-8d %-8d %-14s %-12s %s\n",
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
         * Child gets its own process group.
         */
        setpgid(0, 0);

        /*
         * Child receives normal signal behavior.
         */
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent sets process group.
     */
    setpgid(pid, pid);

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
 * Execute foreground process.
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
         * Foreground child gets its own
         * process group.
         */
        setpgid(0, 0);

        /*
         * Child uses default SIGINT behavior.
         */
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    /*
     * Parent creates process group.
     */
    setpgid(pid, pid);

    /*
     * Store foreground PID.
     */
    foreground_pid = pid;

    printf("Foreground process started.\n");
    printf("PID=%d PGID=%d\n",
           pid,
           pid);

    printf("Press Ctrl+C to send SIGINT.\n");

    int status;

    /*
     * Wait for foreground process.
     */
    while (1) {

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

        if (WIFSTOPPED(status)) {

            printf("\nForeground process stopped.\n");

            add_job(
                pid,
                pid,
                command,
                JOB_STOPPED
            );

            break;
        }

        if (WIFEXITED(status)) {

            printf("\nForeground process exited.\n");

            printf("Exit status: %d\n",
                   WEXITSTATUS(status));

            break;
        }

        if (WIFSIGNALED(status)) {

            printf("\nForeground process terminated by %s.\n",
                   signal_name(
                       WTERMSIG(status)
                   ));

            break;
        }
    }

    /*
     * Clear foreground process.
     */
    foreground_pid = 0;

    /*
     * Reset SIGINT flag.
     */
    sigint_received = 0;
}


/*
 * Display signal status.
 */
void display_signal_status(void)
{
    printf("\n");
    printf("========== SIGNAL STATUS ==========\n");

    printf("Shell PID       : %d\n",
           getpid());

    printf("Shell PGID      : %d\n",
           getpgrp());

    printf("Foreground PID  : %d\n",
           foreground_pid);

    printf("SIGINT received : %s\n",
           sigint_received ?
           "YES" : "NO");

    printf("SIGCHLD event   : %s\n",
           child_changed ?
           "YES" : "NO");

    printf("===================================\n");
}


/*
 * Test signal handling.
 */
void test_signals(void)
{
    printf("\n");
    printf("========== SIGNAL TEST ==========\n");

    printf("SIGINT  : Ctrl+C interrupt\n");
    printf("SIGTERM : termination request\n");
    printf("SIGTSTP : terminal stop\n");
    printf("SIGCHLD : child state notification\n");

    printf("\nSignal handlers are registered.\n");
    printf("Shell SIGINT protection is active.\n");

    printf("=================================\n");
}


/*
 * Validate job table.
 */
void validate_jobs(void)
{
    update_job_states();

    printf("\n");
    printf("========== JOB VALIDATION ==========\n");

    if (job_count == 0) {

        printf("Job table is empty.\n");
        printf("Validation: OK\n");

        printf("====================================\n");

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

    printf("====================================\n");
}


/*
 * Cleanup completed jobs.
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
 * Display help.
 */
void show_help(void)
{
    printf("\n");
    printf("Available commands:\n");
    printf("  command &       Run command in background\n");
    printf("  jobs             Display job table\n");
    printf("  signals          Display signal information\n");
    printf("  status           Display signal status\n");
    printf("  validate         Validate job records\n");
    printf("  cleanup          Remove completed jobs\n");
    printf("  test             Test signal handlers\n");
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

    /*
     * Initialize shell.
     */
    initialize_shell();

    /*
     * Register handlers.
     */
    register_signal_handlers();

    printf("============================================================\n");
    printf("          SKILL-21 SIGNAL INTERRUPT MANAGER\n");
    printf("============================================================\n");

    show_help();

    while (1) {

        /*
         * Update jobs after child events.
         */
        if (child_changed) {

            update_job_states();

            child_changed = 0;
        }

        printf("skill21$ ");

        fflush(stdout);

        /*
         * Read command.
         */
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
         * jobs.
         */
        if (strcmp(line, "jobs") == 0) {

            list_jobs();

            continue;
        }


        /*
         * signals.
         */
        if (strcmp(line, "signals") == 0) {

            printf("\n");
            printf("SIGINT  = %d\n", SIGINT);
            printf("SIGTERM = %d\n", SIGTERM);
            printf("SIGTSTP = %d\n", SIGTSTP);
            printf("SIGCHLD = %d\n", SIGCHLD);

            continue;
        }


        /*
         * status.
         */
        if (strcmp(line, "status") == 0) {

            display_signal_status();

            continue;
        }


        /*
         * validate.
         */
        if (strcmp(line, "validate") == 0) {

            validate_jobs();

            continue;
        }


        /*
         * cleanup.
         */
        if (strcmp(line, "cleanup") == 0) {

            cleanup_jobs();

            continue;
        }


        /*
         * test.
         */
        if (strcmp(line, "test") == 0) {

            test_signals();

            continue;
        }


        /*
         * help.
         */
        if (strcmp(line, "help") == 0) {

            show_help();

            continue;
        }


        /*
         * Detect background command.
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
         * Otherwise execute foreground command.
         */
        launch_foreground(line);
    }


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
     * Reap remaining children.
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


    printf("\nSkill-21 terminated.\n");

    return 0;
}
