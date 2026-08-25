#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define REQUEST_FIFO "request_fifo"
#define RESPONSE_FIFO "response_fifo"
#define SIZE 100

int main()
{
    int request_fd, response_fd;
    char message[SIZE];
    char response[SIZE];

    /* Create named pipes */
    mkfifo(REQUEST_FIFO, 0666);
    mkfifo(RESPONSE_FIFO, 0666);

    printf("Server started...\n");
    printf("Waiting for client messages...\n");

    while (1)
    {
        request_fd = open(REQUEST_FIFO, O_RDONLY);

        if (request_fd == -1)
        {
            perror("open request_fifo");
            return 1;
        }

        int n = read(request_fd, message, SIZE - 1);
        close(request_fd);

        if (n > 0)
        {
            message[n] = '\0';

            printf("Client message: %s\n", message);

            if (strcmp(message, "exit") == 0)
            {
                printf("Server shutting down...\n");
                break;
            }

            snprintf(response, SIZE,
                     "Server processed: %s", message);

            response_fd = open(RESPONSE_FIFO, O_WRONLY);

            if (response_fd == -1)
            {
                perror("open response_fifo");
                return 1;
            }

            write(response_fd, response, strlen(response) + 1);
            close(response_fd);
        }
    }

    unlink(REQUEST_FIFO);
    unlink(RESPONSE_FIFO);

    return 0;
}
