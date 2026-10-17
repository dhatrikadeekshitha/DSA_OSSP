#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child executing ls command\n");
        execlp("ls", "ls", "-l", (char *)NULL);
        perror("exec failed");
        _exit(1);
    } else {
        waitpid(pid, NULL, 0);
        printf("Parent: Child execution completed\n");
    }

    return 0;
}
