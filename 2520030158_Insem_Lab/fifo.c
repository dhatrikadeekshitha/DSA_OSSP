#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main() {
    char *fifo = "/tmp/ossp_fifo";
    char message[] = "Hello from parent process";
    char buffer[100];

    mkfifo(fifo, 0666);

    pid_t pid = fork();

    if (pid == 0) {
        int fd = open(fifo, O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
        close(fd);
        exit(0);
    } else {
        int fd = open(fifo, O_WRONLY);
        write(fd, message, strlen(message) + 1);
        close(fd);
        wait(NULL);
        printf("Parent: Message sent successfully.\n");
    }

    return 0;
}
