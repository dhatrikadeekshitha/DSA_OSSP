/*
 * Skill-25: Large Pipeline Execution and Performance Monitoring
 *
 * Objective 1:
 * Execute Large Pipelines, Launch Multiple Jobs, Measure Stability,
 * Monitor Resources, Detect Failures, Analyze Performance.
 *
 * Objective 2:
 * Document Architecture, Create Flow Diagrams, Explain Execution Paths,
 * Describe Syscalls, Record Design Decisions, Review Documentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define MAX_PIPELINE 2048
#define MAX_COMMAND 512
#define MAX_ARGS 64
#define MAX_JOBS 32
#define MAX_STAGES 16


/* ============================================================
 * MODULE 1: JOB MANAGEMENT
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

    char command[MAX_COMMAND];

    JobState state;

    int exit_status;

    double execution_time;

} Job;


Job job_table[MAX_JOBS];

int job_count = 0;

int next_job_id = 1;


/*
 * Convert job state to string.
 */
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
 * Add a job.
 */
int add_job(pid_t pid,
            const char *command)
{
    if (job_count >= MAX_JOBS) {

        printf("Job table is full.\n");

        return -1;
    }

    job_table[job_count].job_id =
        next_job_id++;

    job_table[job_count].pid =
        pid;

    strncpy(
        job_table[job_count].command,
        command,
        MAX_COMMAND - 1
    );

    job_table[job_count]
        .command[MAX_COMMAND - 1] = '\0';

    job_table[job_count].state =
        JOB_RUNNING;

    job_table[job_count].exit_status =
        0;

    job_table[job_count].execution_time =
        0.0;

    return job_count++;
}


/*
 * Display jobs.
 */
void list_jobs(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                       JOB TABLE\n");
    printf("============================================================\n");

    if (job_count == 0) {

        printf("No jobs available.\n");

        return;
    }

    printf(
        "%-5s %-8s %-14s %-10s %-10s %s\n",
        "ID",
        "PID",
        "STATE",
        "EXIT",
        "TIME",
        "COMMAND"
    );

    printf(
        "------------------------------------------------------------\n"
    );

    for (int i = 0;
         i < job_count;
         i++) {

        printf(
            "%-5d %-8d %-14s %-10d %-10.3f %s\n",
            job_table[i].job_id,
            job_table[i].pid,
            job_state_name(
                job_table[i].state
            ),
            job_table[i].exit_status,
            job_table[i].execution_time,
            job_table[i].command
        );
    }

    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 2: TIME AND RESOURCE MONITORING
 * ============================================================
 */


/*
 * Return current time in seconds.
 */
double current_time(void)
{
    struct timeval tv;

    gettimeofday(
        &tv,
        NULL
    );

    return
        (double)tv.tv_sec +
        (double)tv.tv_usec / 1000000.0;
}


/*
 * Display process resource usage.
 */
void display_resource_usage(void)
{
    struct rusage usage;

    if (getrusage(
            RUSAGE_CHILDREN,
            &usage
        ) == -1) {

        perror("getrusage");

        return;
    }

    printf("\n");
    printf("============================================================\n");
    printf("                  RESOURCE MONITOR\n");
    printf("============================================================\n");

    printf(
        "User CPU time      : %ld.%06ld seconds\n",
        usage.ru_utime.tv_sec,
        usage.ru_utime.tv_usec
    );

    printf(
        "System CPU time    : %ld.%06ld seconds\n",
        usage.ru_stime.tv_sec,
        usage.ru_stime.tv_usec
    );

    printf(
        "Maximum memory     : %ld KB\n",
        usage.ru_maxrss
    );

    printf(
        "Page reclaims      : %ld\n",
        usage.ru_minflt
    );

    printf(
        "Page faults        : %ld\n",
        usage.ru_majflt
    );

    printf(
        "Voluntary switches : %ld\n",
        usage.ru_nvcsw
    );

    printf(
        "Involuntary switches: %ld\n",
        usage.ru_nivcsw
    );

    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 3: PIPELINE PARSER
 * ============================================================
 */


/*
 * Split a pipeline into stages.
 */
int parse_pipeline(
    char *input,
    char *stages[])
{
    int count = 0;

    char *token;


    token = strtok(
        input,
        "|"
    );


    while (token != NULL) {

        if (count >= MAX_STAGES) {

            printf(
                "Error: Pipeline has too many stages.\n"
            );

            return -1;
        }


        while (*token == ' ' ||
               *token == '\t') {

            token++;
        }


        stages[count++] =
            token;


        token = strtok(
            NULL,
            "|"
        );
    }


    return count;
}


/*
 * Parse command arguments.
 */
int parse_arguments(
    char *command,
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

            printf(
                "Error: Too many arguments.\n"
            );

            return -1;
        }


        args[argc++] =
            token;


        token = strtok(
            NULL,
            " \t\n"
        );
    }


    args[argc] = NULL;


    return argc;
}


/* ============================================================
 * MODULE 4: PIPELINE EXECUTION
 * ============================================================
 */


/*
 * Execute a multi-stage pipeline.
 *
 * Example:
 *
 *   ls | grep .c | wc -l
 */
int execute_pipeline(
    const char *pipeline)
{
    char pipeline_copy[MAX_PIPELINE];

    char *stages[MAX_STAGES];

    int stage_count;


    strncpy(
        pipeline_copy,
        pipeline,
        MAX_PIPELINE - 1
    );

    pipeline_copy[MAX_PIPELINE - 1] =
        '\0';


    stage_count =
        parse_pipeline(
            pipeline_copy,
            stages
        );


    if (stage_count <= 0) {

        printf(
            "Invalid pipeline.\n"
        );

        return -1;
    }


    printf(
        "\nPipeline stages: %d\n",
        stage_count
    );


    int pipes[MAX_STAGES - 1][2];

    pid_t children[MAX_STAGES];


    /*
     * Create all required pipes.
     */
    for (int i = 0;
         i < stage_count - 1;
         i++) {

        if (pipe(
                pipes[i]
            ) == -1) {

            perror("pipe");

            return -1;
        }
    }


    double start =
        current_time();


    /*
     * Launch pipeline stages.
     */
    for (int i = 0;
         i < stage_count;
         i++) {

        char *args[MAX_ARGS];


        if (parse_arguments(
                stages[i],
                args
            ) <= 0) {

            printf(
                "Invalid stage %d.\n",
                i + 1
            );

            return -1;
        }


        children[i] =
            fork();


        if (children[i] < 0) {

            perror("fork");

            return -1;
        }


        if (children[i] == 0) {

            /*
             * Connect input from previous pipe.
             */
            if (i > 0) {

                if (dup2(
                        pipes[i - 1][0],
                        STDIN_FILENO
                    ) == -1) {

                    perror("dup2");

                    exit(EXIT_FAILURE);
                }
            }


            /*
             * Connect output to next pipe.
             */
            if (i < stage_count - 1) {

                if (dup2(
                        pipes[i][1],
                        STDOUT_FILENO
                    ) == -1) {

                    perror("dup2");

                    exit(EXIT_FAILURE);
                }
            }


            /*
             * Close all pipe descriptors.
             */
            for (int j = 0;
                 j < stage_count - 1;
                 j++) {

                close(
                    pipes[j][0]
                );

                close(
                    pipes[j][1]
                );
            }


            /*
             * Execute stage.
             */
            execvp(
                args[0],
                args
            );


            perror(
                "execvp"
            );

            exit(127);
        }
    }


    /*
     * Parent closes all pipe descriptors.
     */
    for (int i = 0;
         i < stage_count - 1;
         i++) {

        close(
            pipes[i][0]
        );

        close(
            pipes[i][1]
        );
    }


    /*
     * Wait for all pipeline stages.
     */
    int final_status = 0;


    for (int i = 0;
         i < stage_count;
         i++) {

        int status;


        if (waitpid(
                children[i],
                &status,
                0
            ) == -1) {

            perror(
                "waitpid"
            );

            final_status =
                1;

            continue;
        }


        if (WIFEXITED(status)) {

            int exit_code =
                WEXITSTATUS(status);


            printf(
                "Stage %d exit status: %d\n",
                i + 1,
                exit_code
            );


            if (exit_code != 0)
                final_status =
                    exit_code;
        }
        else if (WIFSIGNALED(status)) {

            printf(
                "Stage %d terminated by signal %d\n",
                i + 1,
                WTERMSIG(status)
            );

            final_status =
                1;
        }
    }


    double end =
        current_time();


    printf(
        "Pipeline execution time: %.6f seconds\n",
        end - start
    );


    if (final_status == 0) {

        printf(
            "Pipeline result: SUCCESS\n"
        );
    }
    else {

        printf(
            "Pipeline result: FAILURE\n"
        );
    }


    return final_status;
}


/* ============================================================
 * MODULE 5: MULTIPLE JOB EXECUTION
 * ============================================================
 */


/*
 * Launch a background command.
 */
int launch_background_job(
    const char *command)
{
    pid_t pid;


    pid =
        fork();


    if (pid < 0) {

        perror(
            "fork"
        );

        return -1;
    }


    if (pid == 0) {

        char *args[MAX_ARGS];

        char command_copy[MAX_COMMAND];


        strncpy(
            command_copy,
            command,
            MAX_COMMAND - 1
        );

        command_copy[
            MAX_COMMAND - 1
        ] = '\0';


        if (parse_arguments(
                command_copy,
                args
            ) <= 0) {

            exit(EXIT_FAILURE);
        }


        execvp(
            args[0],
            args
        );


        perror(
            "execvp"
        );

        exit(127);
    }


    int index =
        add_job(
            pid,
            command
        );


    if (index == -1) {

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


/*
 * Update all background jobs.
 */
void update_jobs(void)
{
    for (int i = 0;
         i < job_count;
         i++) {

        if (job_table[i].state !=
            JOB_RUNNING)
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


        if (result == -1)
            continue;


        if (WIFEXITED(status)) {

            job_table[i].exit_status =
                WEXITSTATUS(status);


            if (job_table[i].exit_status == 0) {

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


/* ============================================================
 * MODULE 6: STABILITY TEST
 * ============================================================
 */


/*
 * Launch multiple simple jobs.
 */
void stability_test(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    STABILITY TEST\n");
    printf("============================================================\n");


    const char *commands[] = {

        "sleep 2",

        "sleep 3",

        "echo Job-A",

        "echo Job-B",

        "echo Job-C"

    };


    int count =
        sizeof(commands) /
        sizeof(commands[0]);


    for (int i = 0;
         i < count;
         i++) {

        launch_background_job(
            commands[i]
        );
    }


    printf(
        "\nMultiple jobs launched successfully.\n"
    );


    printf(
        "Use 'jobs' to monitor their state.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 7: FAILURE DETECTION
 * ============================================================
 */


/*
 * Test a pipeline containing a failing command.
 */
void failure_test(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    FAILURE TEST\n");
    printf("============================================================\n");


    printf(
        "Testing pipeline with invalid command...\n"
    );


    execute_pipeline(
        "echo hello | command_that_does_not_exist | wc -l"
    );


    printf(
        "Failure analysis completed.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 8: PERFORMANCE ANALYSIS
 * ============================================================
 */


/*
 * Run a pipeline and measure its execution time.
 */
void performance_test(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                 PERFORMANCE TEST\n");
    printf("============================================================\n");


    double start =
        current_time();


    int status =
        execute_pipeline(
            "printf 'one\\ntwo\\nthree\\nfour\\n' | wc -l"
        );


    double end =
        current_time();


    printf(
        "\nTotal measured time: %.6f seconds\n",
        end - start
    );


    printf(
        "Final pipeline status: %d\n",
        status
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 9: ARCHITECTURE DOCUMENTATION
 * ============================================================
 */


/*
 * Display architecture.
 */
void show_architecture(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                 SYSTEM ARCHITECTURE\n");
    printf("============================================================\n");


    printf(
        "\n"
        "                     USER COMMAND\n"
        "                           |\n"
        "                           v\n"
        "                  COMMAND PARSER\n"
        "                           |\n"
        "             +-------------+-------------+\n"
        "             |                           |\n"
        "             v                           v\n"
        "       PIPELINE MODULE             JOB MODULE\n"
        "             |                           |\n"
        "             v                           v\n"
        "      pipe() / fork()             fork() / execvp()\n"
        "             |                           |\n"
        "             v                           v\n"
        "         dup2()                     PROCESS\n"
        "             |\n"
        "             v\n"
        "          execvp()\n"
        "             |\n"
        "             v\n"
        "       waitpid() / getrusage()\n"
        "             |\n"
        "             v\n"
        "      PERFORMANCE REPORT\n"
    );


    printf(
        "\n============================================================\n"
    );
}


/*
 * Display execution path.
 */
void show_execution_path(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    EXECUTION PATH\n");
    printf("============================================================\n");


    printf(
        "\n1. Read user command\n"
    );

    printf(
        "2. Parse pipeline stages\n"
    );

    printf(
        "3. Create pipes between stages\n"
    );

    printf(
        "4. Fork child processes\n"
    );

    printf(
        "5. Connect stdin/stdout using dup2()\n"
    );

    printf(
        "6. Execute commands using execvp()\n"
    );

    printf(
        "7. Close unused file descriptors\n"
    );

    printf(
        "8. Wait for child processes using waitpid()\n"
    );

    printf(
        "9. Check exit status and failures\n"
    );

    printf(
        "10. Measure execution time\n"
    );

    printf(
        "11. Monitor resource usage\n"
    );

    printf(
        "12. Generate performance result\n"
    );


    printf(
        "\n============================================================\n"
    );
}


/*
 * Display system calls.
 */
void show_syscalls(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    SYSTEM CALLS\n");
    printf("============================================================\n");


    printf(
        "fork()       : Creates child processes\n"
    );

    printf(
        "pipe()       : Creates communication channels\n"
    );

    printf(
        "dup2()       : Redirects standard input/output\n"
    );

    printf(
        "execvp()     : Executes a command\n"
    );

    printf(
        "waitpid()    : Waits for child processes\n"
    );

    printf(
        "getrusage()  : Collects resource usage\n"
    );

    printf(
        "gettimeofday(): Measures execution time\n"
    );

    printf(
        "kill()       : Sends signals to processes\n"
    );

    printf(
        "close()      : Releases file descriptors\n"
    );


    printf(
        "============================================================\n"
    );
}


/*
 * Display design decisions.
 */
void show_design_decisions(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    DESIGN DECISIONS\n");
    printf("============================================================\n");


    printf(
        "1. Pipelines are limited to 16 stages.\n"
    );

    printf(
        "2. Each pipeline stage executes in a separate child process.\n"
    );

    printf(
        "3. Pipes connect stdout of one stage to stdin of the next.\n"
    );

    printf(
        "4. Unused file descriptors are closed immediately.\n"
    );

    printf(
        "5. Parent waits for all pipeline processes.\n"
    );

    printf(
        "6. Background jobs are stored in a job table.\n"
    );

    printf(
        "7. Exit statuses are used to detect failures.\n"
    );

    printf(
        "8. gettimeofday() measures elapsed execution time.\n"
    );

    printf(
        "9. getrusage() monitors child resource consumption.\n"
    );

    printf(
        "10. Modular functions separate parsing, execution,\n"
        "    monitoring and documentation responsibilities.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 10: HELP
 * ============================================================
 */

void show_help(void)
{
    printf("\n");

    printf(
        "Available commands:\n"
    );

    printf(
        "  pipeline <cmd>    Execute a pipeline\n"
    );

    printf(
        "  jobs              Display job table\n"
    );

    printf(
        "  stability         Launch multiple jobs\n"
    );

    printf(
        "  resources         Display resource usage\n"
    );

    printf(
        "  performance       Run performance test\n"
    );

    printf(
        "  failure           Test failure detection\n"
    );

    printf(
        "  architecture      Display architecture diagram\n"
    );

    printf(
        "  path              Display execution path\n"
    );

    printf(
        "  syscalls          Display system calls\n"
    );

    printf(
        "  decisions         Display design decisions\n"
    );

    printf(
        "  help              Display help\n"
    );

    printf(
        "  exit              Exit program\n"
    );

    printf("\n");
}


/* ============================================================
 * MAIN PROGRAM
 * ============================================================
 */

int main(void)
{
    char line[MAX_PIPELINE];


    printf(
        "============================================================\n"
    );

    printf(
        "       SKILL-25 PIPELINE & PERFORMANCE MANAGER\n"
    );

    printf(
        "============================================================\n"
    );


    show_help();


    while (1) {

        update_jobs();


        printf(
            "skill25$ "
        );

        fflush(stdout);


        if (fgets(
                line,
                sizeof(line),
                stdin
            ) == NULL) {

            break;
        }


        line[
            strcspn(
                line,
                "\n"
            )
        ] = '\0';


        if (strlen(line) == 0)
            continue;


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
         * Jobs.
         */
        if (strcmp(
                line,
                "jobs"
            ) == 0) {

            list_jobs();

            continue;
        }


        /*
         * Resource monitoring.
         */
        if (strcmp(
                line,
                "resources"
            ) == 0) {

            display_resource_usage();

            continue;
        }


        /*
         * Stability test.
         */
        if (strcmp(
                line,
                "stability"
            ) == 0) {

            stability_test();

            continue;
        }


        /*
         * Performance test.
         */
        if (strcmp(
                line,
                "performance"
            ) == 0) {

            performance_test();

            continue;
        }


        /*
         * Failure test.
         */
        if (strcmp(
                line,
                "failure"
            ) == 0) {

            failure_test();

            continue;
        }


        /*
         * Architecture.
         */
        if (strcmp(
                line,
                "architecture"
            ) == 0) {

            show_architecture();

            continue;
        }


        /*
         * Execution path.
         */
        if (strcmp(
                line,
                "path"
            ) == 0) {

            show_execution_path();

            continue;
        }


        /*
         * System calls.
         */
        if (strcmp(
                line,
                "syscalls"
            ) == 0) {

            show_syscalls();

            continue;
        }


        /*
         * Design decisions.
         */
        if (strcmp(
                line,
                "decisions"
            ) == 0) {

            show_design_decisions();

            continue;
        }


        /*
         * Pipeline command.
         */
        if (strncmp(
                line,
                "pipeline ",
                9
            ) == 0) {

            execute_pipeline(
                line + 9
            );

            continue;
        }


        printf(
            "Unknown command: %s\n",
            line
        );

        printf(
            "Type 'help' for available commands.\n"
        );
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


    printf(
        "\nSkill-25 terminated.\n"
    );


    return 0;
}
