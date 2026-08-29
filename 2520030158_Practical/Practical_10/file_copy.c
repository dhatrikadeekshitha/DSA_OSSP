#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define BUFFER_SIZE 4096

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/* Low-level system call copy */
int low_level_copy(const char *source, const char *destination)
{
    int src_fd, dest_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    src_fd = open(source, O_RDONLY);

    if (src_fd == -1)
    {
        perror("open source");
        return -1;
    }

    dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (dest_fd == -1)
    {
        perror("open destination");
        close(src_fd);
        return -1;
    }

    /* Demonstrate lseek() */
    lseek(src_fd, 0, SEEK_SET);

    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0)
    {
        bytes_written = write(dest_fd, buffer, bytes_read);

        if (bytes_written != bytes_read)
        {
            perror("write");
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    close(src_fd);
    close(dest_fd);

    return 0;
}

/* Standard library copy */
int standard_library_copy(const char *source, const char *destination)
{
    FILE *src;
    FILE *dest;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    src = fopen(source, "rb");

    if (src == NULL)
    {
        perror("fopen source");
        return -1;
    }

    dest = fopen(destination, "wb");

    if (dest == NULL)
    {
        perror("fopen destination");
        fclose(src);
        return -1;
    }

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dest);
    }

    fclose(src);
    fclose(dest);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    char *source = argv[1];

    printf("File Copy Performance Comparison\n");
    printf("=================================\n");
    printf("Source file: %s\n", source);

    double start, end;

    /* Low-level copy */
    start = get_time();

    if (low_level_copy(source, "copy_lowlevel.txt") != 0)
    {
        return 1;
    }

    end = get_time();

    printf("\nLow-level system call copy:\n");
    printf("Time: %.6f seconds\n", end - start);

    /* Standard library copy */
    start = get_time();

    if (standard_library_copy(source, "copy_standard.txt") != 0)
    {
        return 1;
    }

    end = get_time();

    printf("\nStandard library copy:\n");
    printf("Time: %.6f seconds\n", end - start);

    printf("\nFiles copied successfully.\n");

    return 0;
}
