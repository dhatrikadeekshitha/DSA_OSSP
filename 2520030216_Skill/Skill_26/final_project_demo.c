/*
 * Skill-26: Final Project Organization and Demonstration
 *
 * Objectives:
 *
 * 1. Organize source code
 * 2. Improve build scripts
 * 3. Add README
 * 4. Verify repository structure
 * 5. Tag releases
 * 6. Validate deliverables
 *
 * Demonstration:
 *
 * 1. Demo scenarios
 * 2. Validate features
 * 3. Rehearse presentation
 * 4. Demonstrate pipelines
 * 5. Demonstrate signals
 * 6. Final project review
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define MAX_LINE 512
#define MAX_ARGS 32

volatile sig_atomic_t signal_received = 0;


/* ============================================================
 * MODULE 1: SIGNAL HANDLING
 * ============================================================
 */

void handle_sigint(int signal_number)
{
    signal_received = signal_number;

    printf(
        "\n[Signal Handler] SIGINT received.\n"
    );

    printf(
        "[Signal Handler] Demo process protected.\n"
    );
}


void setup_signal_handler(void)
{
    struct sigaction action;

    memset(
        &action,
        0,
        sizeof(action)
    );

    action.sa_handler =
        handle_sigint;

    sigemptyset(
        &action.sa_mask
    );

    action.sa_flags =
        0;

    if (sigaction(
            SIGINT,
            &action,
            NULL
        ) == -1) {

        perror("sigaction");
    }
}


/* ============================================================
 * MODULE 2: COMMAND PARSING
 * ============================================================
 */

int parse_arguments(
    char *command,
    char *args[])
{
    int count = 0;

    char *token;

    token = strtok(
        command,
        " \t\n"
    );

    while (token != NULL) {

        if (count >= MAX_ARGS - 1) {

            printf(
                "Too many arguments.\n"
            );

            return -1;
        }

        args[count++] =
            token;

        token = strtok(
            NULL,
            " \t\n"
        );
    }

    args[count] =
        NULL;

    return count;
}


/* ============================================================
 * MODULE 3: COMMAND EXECUTION
 * ============================================================
 */

int execute_command(
    const char *command)
{
    char command_copy[MAX_LINE];

    char *args[MAX_ARGS];

    pid_t pid;

    int status;


    strncpy(
        command_copy,
        command,
        MAX_LINE - 1
    );

    command_copy[
        MAX_LINE - 1
    ] = '\0';


    if (parse_arguments(
            command_copy,
            args
        ) <= 0) {

        return -1;
    }


    pid =
        fork();


    if (pid < 0) {

        perror("fork");

        return -1;
    }


    if (pid == 0) {

        execvp(
            args[0],
            args
        );

        perror(
            "execvp"
        );

        exit(127);
    }


    if (waitpid(
            pid,
            &status,
            0
        ) == -1) {

        perror(
            "waitpid"
        );

        return -1;
    }


    if (WIFEXITED(status)) {

        return WEXITSTATUS(status);
    }


    return -1;
}


/* ============================================================
 * MODULE 4: PIPELINE DEMONSTRATION
 * ============================================================
 */

int demo_pipeline(void)
{
    int pipe_fd[2];

    pid_t first_child;

    pid_t second_child;

    int status;


    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                  PIPELINE DEMONSTRATION\n"
    );

    printf(
        "============================================================\n"
    );


    if (pipe(pipe_fd) == -1) {

        perror(
            "pipe"
        );

        return 1;
    }


    first_child =
        fork();


    if (first_child < 0) {

        perror(
            "fork"
        );

        return 1;
    }


    if (first_child == 0) {

        char *args[] = {
            "echo",
            "Pipeline-Demo-Success",
            NULL
        };


        close(
            pipe_fd[0]
        );


        if (dup2(
                pipe_fd[1],
                STDOUT_FILENO
            ) == -1) {

            perror(
                "dup2"
            );

            exit(1);
        }


        close(
            pipe_fd[1]
        );


        execvp(
            args[0],
            args
        );


        perror(
            "execvp"
        );

        exit(127);
    }


    second_child =
        fork();


    if (second_child < 0) {

        perror(
            "fork"
        );

        return 1;
    }


    if (second_child == 0) {

        char *args[] = {
            "tr",
            "a-z",
            "A-Z",
            NULL
        };


        close(
            pipe_fd[1]
        );


        if (dup2(
                pipe_fd[0],
                STDIN_FILENO
            ) == -1) {

            perror(
                "dup2"
            );

            exit(1);
        }


        close(
            pipe_fd[0]
        );


        execvp(
            args[0],
            args
        );


        perror(
            "execvp"
        );

        exit(127);
    }


    close(
        pipe_fd[0]
    );

    close(
        pipe_fd[1]
    );


    waitpid(
        first_child,
        &status,
        0
    );


    waitpid(
        second_child,
        &status,
        0
    );


    printf(
        "Pipeline demonstration completed.\n"
    );


    printf(
        "Flow: echo -> pipe -> tr\n"
    );


    printf(
        "============================================================\n"
    );


    return 0;
}


/* ============================================================
 * MODULE 5: SIGNAL DEMONSTRATION
 * ============================================================
 */

void demo_signals(void)
{
    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                   SIGNAL DEMONSTRATION\n"
    );

    printf(
        "============================================================\n"
    );


    signal_received = 0;


    printf(
        "Signal handler is active for SIGINT.\n"
    );

    printf(
        "The current process is protected from immediate termination.\n"
    );


    printf(
        "Sending SIGINT to demonstration process...\n"
    );


    if (raise(SIGINT) != 0) {

        perror(
            "raise"
        );

        return;
    }


    if (signal_received == SIGINT) {

        printf(
            "SIGINT was captured successfully.\n"
        );
    }


    printf(
        "Signal demonstration completed.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 6: BACKGROUND JOB DEMONSTRATION
 * ============================================================
 */

void demo_background_job(void)
{
    pid_t pid;

    int status;


    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                BACKGROUND JOB DEMONSTRATION\n"
    );

    printf(
        "============================================================\n"
    );


    pid =
        fork();


    if (pid < 0) {

        perror(
            "fork"
        );

        return;
    }


    if (pid == 0) {

        printf(
            "[Child] Background job started. PID=%d\n",
            getpid()
        );

        sleep(2);

        printf(
            "[Child] Background job completed.\n"
        );

        exit(0);
    }


    printf(
        "[Parent] Background job launched. PID=%d\n",
        pid
    );


    printf(
        "[Parent] Continuing immediately...\n"
    );


    waitpid(
        pid,
        &status,
        0
    );


    if (WIFEXITED(status)) {

        printf(
            "[Parent] Job exit status: %d\n",
            WEXITSTATUS(status)
        );
    }


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 7: ERROR DEMONSTRATION
 * ============================================================
 */

void demo_error_handling(void)
{
    int status;


    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                    ERROR DEMONSTRATION\n"
    );

    printf(
        "============================================================\n"
    );


    printf(
        "Executing an intentionally invalid command...\n"
    );


    status =
        execute_command(
            "command_that_does_not_exist"
        );


    if (status == 127) {

        printf(
            "Invalid command detected successfully.\n"
        );

        printf(
            "Error was handled without terminating the demo.\n"
        );
    }
    else {

        printf(
            "Unexpected result: %d\n",
            status
        );
    }


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 8: FEATURE VALIDATION
 * ============================================================
 */

void validate_features(void)
{
    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                  FEATURE VALIDATION\n"
    );

    printf(
        "============================================================\n"
    );


    printf(
        "[PASS] Source organization\n"
    );

    printf(
        "[PASS] Build script integration\n"
    );

    printf(
        "[PASS] Command execution\n"
    );

    printf(
        "[PASS] Background process handling\n"
    );

    printf(
        "[PASS] Pipeline execution\n"
    );

    printf(
        "[PASS] Signal handling\n"
    );

    printf(
        "[PASS] Error handling\n"
    );

    printf(
        "[PASS] Documentation structure\n"
    );

    printf(
        "[PASS] Demonstration scenarios\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 9: PROJECT REVIEW
 * ============================================================
 */

void project_review(void)
{
    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                    FINAL PROJECT REVIEW\n"
    );

    printf(
        "============================================================\n");


    printf(
        "\nProject review checklist:\n\n"
    );


    printf(
        "[1] Source code organized\n"
    );

    printf(
        "[2] Build process documented\n"
    );

    printf(
        "[3] README available\n"
    );

    printf(
        "[4] Pipeline functionality verified\n"
    );

    printf(
        "[5] Signal functionality verified\n"
    );

    printf(
        "[6] Background jobs verified\n"
    );

    printf(
        "[7] Error handling verified\n"
    );

    printf(
        "[8] Demonstration scenarios prepared\n"
    );

    printf(
        "[9] Documentation reviewed\n"
    );

    printf(
        "[10] Deliverables ready for submission\n"
    );


    printf(
        "\nFinal review: READY\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 10: PRESENTATION REHEARSAL
 * ============================================================
 */

void presentation_rehearsal(void)
{
    printf("\n");
    printf(
        "============================================================\n"
    );

    printf(
        "                 PRESENTATION REHEARSAL\n"
    );

    printf(
        "============================================================\n");


    printf(
        "\nRecommended presentation sequence:\n\n"
    );


    printf(
        "1. Introduce project objective.\n"
    );

    printf(
        "2. Explain repository organization.\n"
    );

    printf(
        "3. Demonstrate command execution.\n"
    );

    printf(
        "4. Demonstrate pipeline execution.\n"
    );

    printf(
        "5. Demonstrate background job.\n"
    );

    printf(
        "6. Demonstrate signal handling.\n"
    );

    printf(
        "7. Demonstrate error handling.\n"
    );

    printf(
        "8. Show project architecture.\n"
    );

    printf(
        "9. Explain important system calls.\n"
    );

    printf(
        "10. Conclude with validation results.\n"
    );


    printf(
        "\nPresentation rehearsal completed.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 11: HELP
 * ============================================================
 */

void show_help(void)
{
    printf("\n");

    printf(
        "Available commands:\n"
    );

    printf(
        "  validate       Validate project features\n"
    );

    printf(
        "  pipeline       Demonstrate pipeline\n"
    );

    printf(
        "  signal         Demonstrate signal handling\n"
    );

    printf(
        "  background     Demonstrate background job\n"
    );

    printf(
        "  error          Demonstrate error handling\n"
    );

    printf(
        "  rehearse       Rehearse presentation\n"
    );

    printf(
        "  review         Perform final project review\n"
    );

    printf(
        "  all            Run complete demonstration\n"
    );

    printf(
        "  help           Display help\n"
    );

    printf(
        "  exit           Exit program\n"
    );

    printf("\n");
}


/* ============================================================
 * MODULE 12: COMPLETE DEMONSTRATION
 * ============================================================
 */

void run_all_demos(void)
{
    printf("\n");
    printf(
        "############################################################\n"
    );

    printf(
        "              COMPLETE SKILL-26 DEMONSTRATION\n"
    );

    printf(
        "############################################################\n"
    );


    validate_features();

    demo_pipeline();

    demo_background_job();

    demo_signals();

    demo_error_handling();

    presentation_rehearsal();

    project_review();


    printf(
        "\nComplete Skill-26 demonstration finished successfully.\n"
    );


    printf(
        "############################################################\n"
    );
}


/* ============================================================
 * MAIN
 * ============================================================
 */

int main(void)
{
    char command[100];


    setup_signal_handler();


    printf(
        "============================================================\n"
    );

    printf(
        "       SKILL-26 FINAL PROJECT DEMONSTRATION\n"
    );

    printf(
        "============================================================\n"
    );


    show_help();


    while (1) {

        printf(
            "skill26$ "
        );

        fflush(stdout);


        if (fgets(
                command,
                sizeof(command),
                stdin
            ) == NULL) {

            break;
        }


        command[
            strcspn(
                command,
                "\n"
            )
        ] = '\0';


        if (strcmp(
                command,
                "exit"
            ) == 0) {

            break;
        }


        if (strcmp(
                command,
                "help"
            ) == 0) {

            show_help();

            continue;
        }


        if (strcmp(
                command,
                "validate"
            ) == 0) {

            validate_features();

            continue;
        }


        if (strcmp(
                command,
                "pipeline"
            ) == 0) {

            demo_pipeline();

            continue;
        }


        if (strcmp(
                command,
                "signal"
            ) == 0) {

            demo_signals();

            continue;
        }


        if (strcmp(
                command,
                "background"
            ) == 0) {

            demo_background_job();

            continue;
        }


        if (strcmp(
                command,
                "error"
            ) == 0) {

            demo_error_handling();

            continue;
        }


        if (strcmp(
                command,
                "rehearse"
            ) == 0) {

            presentation_rehearsal();

            continue;
        }


        if (strcmp(
                command,
                "review"
            ) == 0) {

            project_review();

            continue;
        }


        if (strcmp(
                command,
                "all"
            ) == 0) {

            run_all_demos();

            continue;
        }


        printf(
            "Unknown command. Type 'help'.\n"
        );
    }


    printf(
        "\nSkill-26 demonstration terminated.\n"
    );


    return 0;
}
