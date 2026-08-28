#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int input_fd;
    int output_fd;

    printf("I/O Redirection using dup2()\n");
    printf("============================\n");

    /*
     * Open input file.
     * This file will become standard input.
     */
    input_fd = open("input.txt", O_RDONLY);

    if (input_fd == -1)
    {
        perror("open input.txt");
        return 1;
    }

    /*
     * Open output file.
     * This file will become standard output.
     */
    output_fd = open("output.txt",
                     O_WRONLY | O_CREAT | O_TRUNC,
                     0644);

    if (output_fd == -1)
    {
        perror("open output.txt");
        close(input_fd);
        return 1;
    }

    /*
     * Redirect standard input (file descriptor 0)
     * to input.txt.
     */
    if (dup2(input_fd, STDIN_FILENO) == -1)
    {
        perror("dup2 stdin");
        return 1;
    }

    /*
     * Redirect standard output (file descriptor 1)
     * to output.txt.
     */
    if (dup2(output_fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 stdout");
        return 1;
    }

    close(input_fd);
    close(output_fd);

    /*
     * From this point:
     * scanf() reads from input.txt
     * printf() writes to output.txt
     */
    char message[100];

    scanf("%99[^\n]", message);

    printf("Data read using redirected stdin: %s\n", message);

    return 0;
}
