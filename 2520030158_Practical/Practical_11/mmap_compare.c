#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

#define BUFFER_SIZE 4096

double get_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/* Traditional read/write file copy */
int traditional_copy(const char *source, const char *destination)
{
    int src_fd;
    int dest_fd;

    char buffer[BUFFER_SIZE];

    ssize_t bytes_read;
    ssize_t bytes_written;

    src_fd = open(source, O_RDONLY);

    if (src_fd == -1)
    {
        perror("open source");
        return -1;
    }

    dest_fd = open(destination,
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644);

    if (dest_fd == -1)
    {
        perror("open destination");
        close(src_fd);
        return -1;
    }

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

/* Memory-mapped file copy */
int mmap_copy(const char *source, const char *destination)
{
    int src_fd;
    int dest_fd;

    struct stat file_stat;

    char *src_map;
    char *dest_map;

    src_fd = open(source, O_RDONLY);

    if (src_fd == -1)
    {
        perror("open source");
        return -1;
    }

    if (fstat(src_fd, &file_stat) == -1)
    {
        perror("fstat");
        close(src_fd);
        return -1;
    }

    size_t file_size = file_stat.st_size;

    dest_fd = open(destination,
                   O_RDWR | O_CREAT | O_TRUNC,
                   0644);

    if (dest_fd == -1)
    {
        perror("open destination");
        close(src_fd);
        return -1;
    }

    if (ftruncate(dest_fd, file_size) == -1)
    {
        perror("ftruncate");
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    if (file_size == 0)
    {
        close(src_fd);
        close(dest_fd);
        return 0;
    }

    src_map = mmap(NULL,
                   file_size,
                   PROT_READ,
                   MAP_PRIVATE,
                   src_fd,
                   0);

    if (src_map == MAP_FAILED)
    {
        perror("mmap source");
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    dest_map = mmap(NULL,
                    file_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    dest_fd,
                    0);

    if (dest_map == MAP_FAILED)
    {
        perror("mmap destination");
        munmap(src_map, file_size);
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    memcpy(dest_map, src_map, file_size);

    if (msync(dest_map, file_size, MS_SYNC) == -1)
    {
        perror("msync");
    }

    munmap(src_map, file_size);
    munmap(dest_map, file_size);

    close(src_fd);
    close(dest_fd);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    const char *source = argv[1];

    printf("Memory-Mapped I/O vs read/write\n");
    printf("================================\n");
    printf("Source file: %s\n", source);

    double start;
    double end;

    /* Traditional read/write */
    start = get_time();

    if (traditional_copy(source, "copy_readwrite.txt") != 0)
    {
        return 1;
    }

    end = get_time();

    printf("\nTraditional read/write:\n");
    printf("Time: %.6f seconds\n", end - start);

    /* mmap */
    start = get_time();

    if (mmap_copy(source, "copy_mmap.txt") != 0)
    {
        return 1;
    }

    end = get_time();

    printf("\nMemory-mapped I/O:\n");
    printf("Time: %.6f seconds\n", end - start);

    printf("\nBoth files copied successfully.\n");

    return 0;
}
