#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define REQUEST_FIFO "request_fifo"
#define RESPONSE_FIFO "response_fifo"
#define SIZE 100

int main()
{
    int request_fd, response_fd;
    char message[SIZE];
    char response[SIZE];

    printf("Client started.\n");
    printf("Enter message: ");

    fgets(message, SIZE, stdin);

    message[strcspn(message, "\n")] = '\0';

    request_fd = open(REQUEST_FIFO, O_WRONLY);

    if (request_fd == -1)
    {
        perror("open request_fifo");
        return 1;
    }

    write(request_fd, message, strlen(message) + 1);
    close(request_fd);

    response_fd = open(RESPONSE_FIFO, O_RDONLY);

    if (response_fd == -1)
    {
        perror("open response_fifo");
        return 1;
    }

    int n = read(response_fd, response, SIZE - 1);
    close(response_fd);

    if (n > 0)
    {
        response[n] = '\0';
        printf("Server response: %s\n", response);
    }

    return 0;
}
