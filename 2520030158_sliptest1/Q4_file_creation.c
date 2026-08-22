#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    int fd;

    fd = open("assignment.txt", O_CREAT | O_WRONLY, 0644);

    if (fd < 0) {
        perror("File creation failed");
        return 1;
    }

    printf("File assignment.txt created successfully.\n");

    close(fd);

    return 0;
}
