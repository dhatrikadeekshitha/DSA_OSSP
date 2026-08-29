#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define INITIAL_BUFFER_SIZE 16

struct termios original_terminal;

/* -----------------------------
   Command History Linked List
   ----------------------------- */

typedef struct HistoryNode
{
    char *command;
    struct HistoryNode *next;
} HistoryNode;

HistoryNode *history_head = NULL;
HistoryNode *history_tail = NULL;

int history_count = 0;

/* -----------------------------
   Terminal Handling
   ----------------------------- */

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

/* -----------------------------
   Clear Current Input
   ----------------------------- */

void clear_input(char *buffer, size_t *length)
{
    while (*length > 0)
    {
        printf("\b \b");
        (*length)--;
    }

    buffer[0] = '\0';
    fflush(stdout);
}

/* -----------------------------
   Add Command to History
   ----------------------------- */

void add_history(const char *command)
{
    if (strlen(command) == 0)
        return;

    HistoryNode *new_node =
        (HistoryNode *)malloc(sizeof(HistoryNode));

    if (new_node == NULL)
    {
        perror("malloc");
        return;
    }

    new_node->command =
        (char *)malloc(strlen(command) + 1);

    if (new_node->command == NULL)
    {
        perror("malloc");
        free(new_node);
        return;
    }

    strcpy(new_node->command, command);
    new_node->next = NULL;

    if (history_head == NULL)
    {
        history_head = new_node;
        history_tail = new_node;
    }
    else
    {
        history_tail->next = new_node;
        history_tail = new_node;
    }

    history_count++;
}

/* -----------------------------
   Display History
   ----------------------------- */

void display_history()
{
    HistoryNode *current = history_head;
    int number = 1;

    printf("\n\nCommand History:\n");

    if (current == NULL)
    {
        printf("No commands in history.\n");
        return;
    }

    while (current != NULL)
    {
        printf("%d  %s\n", number, current->command);
        current = current->next;
        number++;
    }
}

/* -----------------------------
   Get History Command
   ----------------------------- */

char *get_history_command(int index)
{
    HistoryNode *current = history_head;
    int count = 0;

    while (current != NULL)
    {
        if (count == index)
            return current->command;

        current = current->next;
        count++;
    }

    return NULL;
}

/* -----------------------------
   Free History
   ----------------------------- */

void free_history()
{
    HistoryNode *current = history_head;

    while (current != NULL)
    {
        HistoryNode *next = current->next;

        free(current->command);
        free(current);

        current = next;
    }

    history_head = NULL;
    history_tail = NULL;
    history_count = 0;
}

/* -----------------------------
   Process Commands
   ----------------------------- */

int process_command(const char *command)
{
    if (strcmp(command, "help") == 0)
    {
        printf("\nAvailable commands:\n");
        printf("help     - Show available commands\n");
        printf("hello    - Display greeting\n");
        printf("history  - Show command history\n");
        printf("status   - Show program status\n");
        printf("exit     - Exit the program\n");
    }
    else if (strcmp(command, "hello") == 0)
    {
        printf("\nHello! Welcome to Skill_05.\n");
    }
    else if (strcmp(command, "history") == 0)
    {
        display_history();
    }
    else if (strcmp(command, "status") == 0)
    {
        printf("\nSkill_05 is running successfully.\n");
        printf("Commands stored in history: %d\n", history_count);
    }
    else if (strcmp(command, "exit") == 0)
    {
        printf("\nExiting Skill_05...\n");
        return 1;
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

    return 0;
}

/* -----------------------------
   Read Command
   ----------------------------- */

char *read_command()
{
    size_t capacity = INITIAL_BUFFER_SIZE;
    size_t length = 0;

    char *buffer = malloc(capacity);

    if (buffer == NULL)
    {
        perror("malloc");
        return NULL;
    }

    buffer[0] = '\0';

    int history_position = history_count;

    while (1)
    {
        int ch = getchar();

        /* Enter */
        if (ch == '\n' || ch == '\r')
        {
            buffer[length] = '\0';
            printf("\n");
            return buffer;
        }

        /* Backspace */
        if (ch == 127 || ch == 8)
        {
            if (length > 0)
            {
                length--;
                buffer[length] = '\0';

                printf("\b \b");
                fflush(stdout);
            }

            continue;
        }

        /* Escape Sequence */
        if (ch == 27)
        {
            int second = getchar();

            if (second == '[')
            {
                int third = getchar();

                /* Up Arrow */
                if (third == 'A')
                {
                    if (history_count > 0 &&
                        history_position > 0)
                    {
                        history_position--;

                        char *previous =
                            get_history_command(history_position);

                        if (previous != NULL)
                        {
                            clear_input(buffer, &length);

                            size_t required =
                                strlen(previous) + 1;

                            if (required > capacity)
                            {
                                char *temp =
                                    realloc(buffer, required);

                                if (temp == NULL)
                                {
                                    free(buffer);
                                    return NULL;
                                }

                                buffer = temp;
                                capacity = required;
                            }

                            strcpy(buffer, previous);
                            length = strlen(buffer);

                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    }
                }

                /* Down Arrow */
                else if (third == 'B')
                {
                    if (history_position < history_count - 1)
                    {
                        history_position++;

                        char *next =
                            get_history_command(history_position);

                        if (next != NULL)
                        {
                            clear_input(buffer, &length);

                            size_t required =
                                strlen(next) + 1;

                            if (required > capacity)
                            {
                                char *temp =
                                    realloc(buffer, required);

                                if (temp == NULL)
                                {
                                    free(buffer);
                                    return NULL;
                                }

                                buffer = temp;
                                capacity = required;
                            }

                            strcpy(buffer, next);
                            length = strlen(buffer);

                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    }
                    else
                    {
                        history_position = history_count;
                        clear_input(buffer, &length);
                    }
                }
            }

            continue;
        }

        /* Ctrl + D */
        if (ch == 4)
        {
            printf("\nExiting Skill_05...\n");
            free(buffer);
            return NULL;
        }

        /* Normal character */
        if (ch >= 32 && ch <= 126)
        {
            /*
             * Resize buffer before adding character.
             * This prevents buffer overflow.
             */
            if (length + 1 >= capacity)
            {
                size_t new_capacity = capacity * 2;

                char *temp =
                    realloc(buffer, new_capacity);

                if (temp == NULL)
                {
                    perror("realloc");
                    free(buffer);
                    return NULL;
                }

                buffer = temp;
                capacity = new_capacity;
            }

            buffer[length] = ch;
            length++;

            buffer[length] = '\0';

            putchar(ch);
            fflush(stdout);
        }
    }
}

/* -----------------------------
   Main
   ----------------------------- */

int main()
{
    enable_raw_mode();

    printf("========================================\n");
    printf("      SKILL_05 COMMAND HISTORY\n");
    printf("========================================\n");

    printf("Features:\n");
    printf("- Dynamic input buffer\n");
    printf("- Automatic buffer resizing\n");
    printf("- Command history\n");
    printf("- Up/Down arrow navigation\n");
    printf("- Escape sequence handling\n");
    printf("- Linked list memory management\n\n");

    printf("Type 'help' for commands.\n\n");

    while (1)
    {
        printf("skill05> ");
        fflush(stdout);

        char *command = read_command();

        if (command == NULL)
            break;

        if (strlen(command) > 0)
        {
            add_history(command);
        }

        int should_exit = process_command(command);

        free(command);

        if (should_exit)
            break;
    }

    free_history();

    return 0;
}
