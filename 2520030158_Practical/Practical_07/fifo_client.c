#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_CLIENT "/tmp/client_fifo"
#define FIFO_SERVER "/tmp/server_fifo"

int main() {
    int server_fd, client_fd;
    char message[100];
    char response[150];

    printf("Client started.\n");

    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);

    message[strcspn(message, "\n")] = '\0';

    // Send message to server
    server_fd = open(FIFO_SERVER, O_WRONLY);

    if (server_fd == -1) {
        perror("Error opening server FIFO");
        exit(1);
    }

    write(server_fd, message, strlen(message) + 1);
    close(server_fd);

    // Exit without waiting for response
    if (strcmp(message, "exit") == 0) {
        return 0;
    }

    // Read server response
    client_fd = open(FIFO_CLIENT, O_RDONLY);

    if (client_fd == -1) {
        perror("Error opening client FIFO");
        exit(1);
    }

    int n = read(client_fd, response, sizeof(response) - 1);
    close(client_fd);

    if (n > 0) {
        response[n] = '\0';
        printf("Server response: %s\n", response);
    }

    return 0;
}
