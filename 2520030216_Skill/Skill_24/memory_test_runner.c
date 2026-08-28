/*
 * Skill-24: Memory Validation and Automated Testing
 *
 * Objective 1:
 * Run Valgrind, Identify Leaks, Fix Memory Errors,
 * Release Resources, Verify Cleanup, Document Findings.
 *
 * Objective 2:
 * Design Test Cases, Create Test Scripts, Automate Execution,
 * Verify Outputs, Compare Results, Generate Reports.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_TESTS 10
#define MAX_OUTPUT 2048
#define MAX_COMMAND 512


/* ============================================================
 * MODULE 1: TEST CASE MANAGEMENT
 * ============================================================
 */

typedef struct {

    int test_id;

    char name[100];

    char command[MAX_COMMAND];

    char expected[MAX_OUTPUT];

    int expected_status;

} TestCase;


TestCase test_cases[MAX_TESTS];

int test_count = 0;


/*
 * Add a test case.
 */
void add_test_case(
    int id,
    const char *name,
    const char *command,
    const char *expected,
    int expected_status)
{
    if (test_count >= MAX_TESTS) {

        printf("Test table is full.\n");

        return;
    }

    test_cases[test_count].test_id = id;

    strncpy(
        test_cases[test_count].name,
        name,
        sizeof(test_cases[test_count].name) - 1
    );

    test_cases[test_count]
        .name[sizeof(test_cases[test_count].name) - 1] = '\0';


    strncpy(
        test_cases[test_count].command,
        command,
        MAX_COMMAND - 1
    );

    test_cases[test_count]
        .command[MAX_COMMAND - 1] = '\0';


    strncpy(
        test_cases[test_count].expected,
        expected,
        MAX_OUTPUT - 1
    );

    test_cases[test_count]
        .expected[MAX_OUTPUT - 1] = '\0';


    test_cases[test_count].expected_status =
        expected_status;


    test_count++;
}


/*
 * Design the test cases.
 */
void design_test_cases(void)
{
    test_count = 0;


    add_test_case(
        1,
        "Basic Echo",
        "echo TEST_SUCCESS",
        "TEST_SUCCESS",
        0
    );


    add_test_case(
        2,
        "Arithmetic",
        "expr 10 + 20",
        "30",
        0
    );


    add_test_case(
        3,
        "Invalid Command",
        "command_that_does_not_exist",
        "",
        127
    );


    add_test_case(
        4,
        "Directory Listing",
        "pwd",
        "",
        0
    );


    add_test_case(
        5,
        "Failure Status",
        "sh -c exit 2",
        "",
        2
    );
}


/*
 * Display test cases.
 */
void display_test_cases(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    TEST CASES\n");
    printf("============================================================\n");


    printf(
        "%-5s %-22s %-35s %-12s\n",
        "ID",
        "NAME",
        "COMMAND",
        "EXPECTED"
    );


    printf(
        "------------------------------------------------------------\n"
    );


    for (int i = 0;
         i < test_count;
         i++) {

        printf(
            "%-5d %-22s %-35s %d\n",
            test_cases[i].test_id,
            test_cases[i].name,
            test_cases[i].command,
            test_cases[i].expected_status
        );
    }


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 2: COMMAND EXECUTION
 * ============================================================
 */


/*
 * Execute a command and capture output.
 */
int execute_command(
    const char *command,
    char *output,
    size_t output_size)
{
    char shell_command[MAX_COMMAND + 50];

    FILE *pipe;

    int status;


    snprintf(
        shell_command,
        sizeof(shell_command),
        "%s 2>&1",
        command
    );


    pipe = popen(
        shell_command,
        "r"
    );


    if (pipe == NULL) {

        snprintf(
            output,
            output_size,
            "popen failed: %s",
            strerror(errno)
        );

        return -1;
    }


    output[0] = '\0';


    while (fgets(
        output + strlen(output),
        output_size - strlen(output),
        pipe
    ) != NULL) {

        if (strlen(output) >= output_size - 1)
            break;
    }


    status = pclose(pipe);


    if (status == -1)
        return -1;


    if (WIFEXITED(status))
        return WEXITSTATUS(status);


    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);


    return -1;
}


/*
 * Normalize output by removing final newline.
 */
void normalize_output(char *output)
{
    size_t length = strlen(output);


    while (length > 0 &&
           (output[length - 1] == '\n' ||
            output[length - 1] == '\r')) {

        output[length - 1] = '\0';

        length--;
    }
}


/*
 * Compare actual output with expected output.
 */
int compare_output(
    const char *actual,
    const char *expected)
{
    if (strlen(expected) == 0)
        return 1;


    if (strcmp(actual, expected) == 0)
        return 1;


    return 0;
}


/* ============================================================
 * MODULE 3: AUTOMATED TESTING
 * ============================================================
 */


typedef struct {

    int test_id;

    char name[100];

    int actual_status;

    int expected_status;

    int output_match;

    int passed;

} TestResult;


TestResult results[MAX_TESTS];

int result_count = 0;


/*
 * Run one test.
 */
void run_single_test(TestCase *test)
{
    char output[MAX_OUTPUT];

    int status;


    printf(
        "\nRunning Test %d: %s\n",
        test->test_id,
        test->name
    );


    printf(
        "Command: %s\n",
        test->command
    );


    status =
        execute_command(
            test->command,
            output,
            sizeof(output)
        );


    normalize_output(output);


    results[result_count].test_id =
        test->test_id;


    strncpy(
        results[result_count].name,
        test->name,
        sizeof(results[result_count].name) - 1
    );


    results[result_count].actual_status =
        status;


    results[result_count].expected_status =
        test->expected_status;


    results[result_count].output_match =
        compare_output(
            output,
            test->expected
        );


    results[result_count].passed =
        (
            status == test->expected_status &&
            results[result_count].output_match
        );


    printf(
        "Actual status   : %d\n",
        status
    );


    printf(
        "Expected status : %d\n",
        test->expected_status
    );


    if (strlen(output) > 0) {

        printf(
            "Output          : %s\n",
            output
        );
    }


    if (results[result_count].passed) {

        printf(
            "Result          : PASS\n"
        );
    }
    else {

        printf(
            "Result          : FAIL\n"
        );
    }


    result_count++;
}


/*
 * Run all tests.
 */
void run_all_tests(void)
{
    result_count = 0;


    printf("\n");
    printf("============================================================\n");
    printf("                 AUTOMATED TEST EXECUTION\n");
    printf("============================================================\n");


    for (int i = 0;
         i < test_count;
         i++) {

        run_single_test(
            &test_cases[i]
        );
    }


    printf("\n");
    printf("All test cases executed.\n");
}


/* ============================================================
 * MODULE 4: TEST REPORT
 * ============================================================
 */


/*
 * Generate test report.
 */
void generate_report(void)
{
    int passed = 0;

    int failed = 0;


    printf("\n");
    printf("============================================================\n");
    printf("                    TEST REPORT\n");
    printf("============================================================\n");


    printf(
        "%-5s %-22s %-12s %-12s %-10s\n",
        "ID",
        "TEST",
        "EXPECTED",
        "ACTUAL",
        "RESULT"
    );


    printf(
        "------------------------------------------------------------\n"
    );


    for (int i = 0;
         i < result_count;
         i++) {

        printf(
            "%-5d %-22s %-12d %-12d %-10s\n",
            results[i].test_id,
            results[i].name,
            results[i].expected_status,
            results[i].actual_status,
            results[i].passed ?
            "PASS" :
            "FAIL"
        );


        if (results[i].passed)
            passed++;
        else
            failed++;
    }


    printf(
        "------------------------------------------------------------\n"
    );


    printf(
        "Total Tests : %d\n",
        result_count
    );


    printf(
        "Passed      : %d\n",
        passed
    );


    printf(
        "Failed      : %d\n",
        failed
    );


    if (failed == 0)
        printf(
            "Overall     : ALL TESTS PASSED\n"
        );
    else
        printf(
            "Overall     : SOME TESTS FAILED\n"
        );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 5: MEMORY MANAGEMENT
 * ============================================================
 */


/*
 * Allocate and release memory correctly.
 *
 * This function is intentionally written to demonstrate
 * proper memory cleanup for Valgrind verification.
 */
int memory_test(void)
{
    int *numbers;

    size_t count = 100;


    numbers =
        malloc(
            count * sizeof(int)
        );


    if (numbers == NULL) {

        printf(
            "Memory allocation failed.\n"
        );

        return 1;
    }


    for (size_t i = 0;
         i < count;
         i++) {

        numbers[i] =
            (int)i;
    }


    printf(
        "Allocated %zu integers.\n",
        count
    );


    printf(
        "First value : %d\n",
        numbers[0]
    );


    printf(
        "Last value  : %d\n",
        numbers[count - 1]
    );


    /*
     * Release allocated memory.
     */
    free(numbers);

    numbers = NULL;


    printf(
        "Memory released successfully.\n"
    );


    return 0;
}


/*
 * Verify memory cleanup.
 */
void verify_memory_cleanup(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                  MEMORY CLEANUP TEST\n");
    printf("============================================================\n");


    if (memory_test() == 0) {

        printf(
            "Memory cleanup test: PASSED\n"
        );
    }
    else {

        printf(
            "Memory cleanup test: FAILED\n"
        );
    }


    printf(
        "Run Valgrind externally to verify that\n"
        "no memory leaks remain.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 6: VALGRIND INFORMATION
 * ============================================================
 */


/*
 * Display Valgrind command.
 */
void show_valgrind_command(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("                   VALGRIND CHECK\n");
    printf("============================================================\n");


    printf(
        "Compile with debug information:\n\n"
    );


    printf(
        "gcc -Wall -Wextra -g memory_test_runner.c "
        "-o memory_test_runner\n"
    );


    printf(
        "\nRun Valgrind with:\n\n"
    );


    printf(
        "valgrind --leak-check=full "
        "--show-leak-kinds=all "
        "--track-origins=yes "
        "./memory_test_runner\n"
    );


    printf(
        "\nExpected memory result:\n"
    );


    printf(
        "All heap blocks were freed -- no leaks are possible.\n"
    );


    printf(
        "============================================================\n"
    );
}


/* ============================================================
 * MODULE 7: ERROR TESTING
 * ============================================================
 */


/*
 * Test invalid command.
 */
void test_invalid_command(void)
{
    char output[MAX_OUTPUT];

    int status;


    printf("\n");
    printf("========== ERROR TEST ==========\n");


    status =
        execute_command(
            "command_that_does_not_exist",
            output,
            sizeof(output)
        );


    normalize_output(output);


    printf(
        "Invalid command status: %d\n",
        status
    );


    if (status == 127) {

        printf(
            "Invalid command handling: PASSED\n"
        );
    }
    else {

        printf(
            "Invalid command handling: FAILED\n"
        );
    }


    printf(
        "================================\n"
    );
}


/* ============================================================
 * MODULE 8: HELP
 * ============================================================
 */

void show_help(void)
{
    printf("\n");

    printf(
        "Available commands:\n"
    );

    printf(
        "  cases       Display test cases\n"
    );

    printf(
        "  run         Run automated tests\n"
    );

    printf(
        "  report      Display test report\n"
    );

    printf(
        "  memory      Run memory cleanup test\n"
    );

    printf(
        "  valgrind    Display Valgrind command\n"
    );

    printf(
        "  error       Test invalid command handling\n"
    );

    printf(
        "  help        Display help\n"
    );

    printf(
        "  exit        Exit program\n"
    );

    printf("\n");
}


/* ============================================================
 * MAIN PROGRAM
 * ============================================================
 */

int main(void)
{
    char command[100];


    printf(
        "============================================================\n"
    );

    printf(
        "       SKILL-24 MEMORY & AUTOMATED TEST MANAGER\n"
    );

    printf(
        "============================================================\n"
    );


    /*
     * Design test cases.
     */
    design_test_cases();


    /*
     * Validate test module.
     */
    printf(
        "Test cases initialized: %d\n",
        test_count
    );


    show_help();


    while (1) {

        printf(
            "skill24$ "
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


        if (strlen(command) == 0)
            continue;


        /*
         * Exit.
         */
        if (strcmp(
                command,
                "exit"
            ) == 0) {

            break;
        }


        /*
         * Help.
         */
        if (strcmp(
                command,
                "help"
            ) == 0) {

            show_help();

            continue;
        }


        /*
         * Display test cases.
         */
        if (strcmp(
                command,
                "cases"
            ) == 0) {

            display_test_cases();

            continue;
        }


        /*
         * Run automated tests.
         */
        if (strcmp(
                command,
                "run"
            ) == 0) {

            run_all_tests();

            continue;
        }


        /*
         * Generate/display report.
         */
        if (strcmp(
                command,
                "report"
            ) == 0) {

            generate_report();

            continue;
        }


        /*
         * Memory cleanup test.
         */
        if (strcmp(
                command,
                "memory"
            ) == 0) {

            verify_memory_cleanup();

            continue;
        }


        /*
         * Valgrind instructions.
         */
        if (strcmp(
                command,
                "valgrind"
            ) == 0) {

            show_valgrind_command();

            continue;
        }


        /*
         * Invalid command test.
         */
        if (strcmp(
                command,
                "error"
            ) == 0) {

            test_invalid_command();

            continue;
        }


        printf(
            "Unknown command: %s\n",
            command
        );

        printf(
            "Type 'help' for available commands.\n"
        );
    }


    printf(
        "\nSkill-24 terminated.\n"
    );


    return 0;
}
