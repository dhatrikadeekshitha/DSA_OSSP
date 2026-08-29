#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int source_fd, destination_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    if (argc != 3)
    {
        printf("Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    /* Open source file for reading */
    source_fd = open(argv[1], O_RDONLY);

    if (source_fd == -1)
    {
        perror("Error opening source file");
        return 1;
    }

    /* Create/open destination file for writing */
    destination_fd = open(argv[2],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

    if (destination_fd == -1)
    {
        perror("Error opening destination file");
        close(source_fd);
        return 1;
    }

    /* Read from source and write to destination */
    while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0)
    {
        bytes_written = write(destination_fd, buffer, bytes_read);

        if (bytes_written != bytes_read)
        {
            perror("Error writing to destination file");
            close(source_fd);
            close(destination_fd);
            return 1;
        }
    }

    if (bytes_read == -1)
    {
        perror("Error reading source file");
        close(source_fd);
        close(destination_fd);
        return 1;
    }

    /* Close both files */
    close(source_fd);
    close(destination_fd);

    printf("File copied successfully.\n");

    return 0;
}
