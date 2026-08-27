#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int source_fd, destination_fd;
    char buffer[1024];
    ssize_t bytes_read, bytes_written;

    source_fd = open("input.txt", O_RDONLY);

    if (source_fd < 0)
    {
        perror("Error opening input file");
        return 1;
    }

    destination_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (destination_fd < 0)
    {
        perror("Error opening output file");
        close(source_fd);
        return 1;
    }

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0)
    {
        bytes_written = write(destination_fd, buffer, bytes_read);

        if (bytes_written != bytes_read)
        {
            perror("Error writing to output file");
            close(source_fd);
            close(destination_fd);
            return 1;
        }
    }

    if (bytes_read < 0)
    {
        perror("Error reading input file");
    }

    close(source_fd);
    close(destination_fd);

    printf("File copied successfully using open(), read(), write(), and close().\n");

    return 0;
}
