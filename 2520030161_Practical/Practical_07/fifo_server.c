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
    char buffer[100];

    // Create FIFOs
    mkfifo(FIFO_SERVER, 0666);
    mkfifo(FIFO_CLIENT, 0666);

    printf("Server started...\n");

    while (1) {
        // Open server FIFO for reading
        server_fd = open(FIFO_SERVER, O_RDONLY);

        if (server_fd == -1) {
            perror("Error opening server FIFO");
            exit(1);
        }

        // Read message from client
        int n = read(server_fd, buffer, sizeof(buffer) - 1);
        close(server_fd);

        if (n > 0) {
            buffer[n] = '\0';

            printf("Client message: %s\n", buffer);

            // Process message
            if (strcmp(buffer, "exit") == 0) {
                printf("Server shutting down...\n");
                break;
            }

            // Send response
            client_fd = open(FIFO_CLIENT, O_WRONLY);

            if (client_fd == -1) {
                perror("Error opening client FIFO");
                exit(1);
            }

            char response[150];
            snprintf(response, sizeof(response),
                     "Server processed: %s", buffer);

            write(client_fd, response, strlen(response) + 1);
            close(client_fd);
        }
    }

    unlink(FIFO_SERVER);
    unlink(FIFO_CLIENT);

    return 0;
}
