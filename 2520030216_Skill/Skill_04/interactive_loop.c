#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

struct termios original_terminal;

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
}

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &original_terminal);

    struct termios raw = original_terminal;

    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    atexit(disable_raw_mode);
}

void process_command(char *command)
{
    if (strcmp(command, "help") == 0)
    {
        printf("\nAvailable commands:\n");
        printf("help   - Show available commands\n");
        printf("hello  - Display greeting\n");
        printf("status - Show program status\n");
        printf("exit   - Exit the program\n");
    }
    else if (strcmp(command, "hello") == 0)
    {
        printf("\nHello! Welcome to Skill_04.\n");
    }
    else if (strcmp(command, "status") == 0)
    {
        printf("\nProgram is running successfully.\n");
    }
    else if (strcmp(command, "exit") == 0)
    {
        printf("\nExiting Skill_04...\n");
    }
    else if (strlen(command) == 0)
    {
        printf("\nNo command entered.\n");
    }
    else
    {
        printf("\nUnknown command: %s\n", command);
        printf("Type 'help' for available commands.\n");
    }
}

int main()
{
    char input[BUFFER_SIZE];
    int length;
    char ch;

    enable_raw_mode();

    printf("=====================================\n");
    printf("       SKILL_04 INTERACTIVE LOOP\n");
    printf("=====================================\n");
    printf("Type 'help' for commands.\n");
    printf("Use Backspace to delete characters.\n");
    printf("Press Enter to execute a command.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("skill04> ");
        fflush(stdout);

        length = 0;

        while (1)
        {
            ch = getchar();

            /* Enter key */
            if (ch == '\n' || ch == '\r')
            {
                input[length] = '\0';
                printf("\n");

                process_command(input);

                if (strcmp(input, "exit") == 0)
                {
                    return 0;
                }

                break;
            }

            /* Backspace key */
            else if (ch == 127 || ch == 8)
            {
                if (length > 0)
                {
                    length--;
                    printf("\b \b");
                    fflush(stdout);
                }
            }

            /* Ctrl + D */
            else if (ch == 4)
            {
                printf("\nExiting Skill_04...\n");
                return 0;
            }

            /* Normal characters */
            else if (ch >= 32 && ch <= 126)
            {
                if (length < BUFFER_SIZE - 1)
                {
                    input[length] = ch;
                    length++;

                    putchar(ch);
                    fflush(stdout);
                }
            }
        }
    }

    return 0;
}
