#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("\nReceived SIGINT (Ctrl+C)\n");
    }
    else if (signal == SIGTERM) {
        printf("\nReceived SIGTERM\n");
    }
    else if (signal == SIGUSR1) {
        printf("\nReceived SIGUSR1\n");
    }

    fflush(stdout);
}

int main() {

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);

    printf("Signal handling program started.\n");
    printf("Process ID: %d\n", getpid());
    printf("Press Ctrl+C to send SIGINT.\n");
    printf("Use another terminal to send SIGTERM or SIGUSR1.\n");

    while (1) {
        printf("Program is running...\n");
        sleep(3);
    }

    return 0;
}
