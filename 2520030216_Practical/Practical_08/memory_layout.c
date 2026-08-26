#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_initialized = 100;
int global_uninitialized;

static int static_global = 200;

void code_function()
{
    printf("\n[CODE SEGMENT]\n");
    printf("Address of code_function : %p\n", (void *)code_function);
}

void display_memory_addresses()
{
    int stack_variable = 10;
    static int static_local = 20;

    int *heap_variable = malloc(sizeof(int));

    if (heap_variable == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *heap_variable = 30;

    printf("\n========== MEMORY ADDRESSES ==========\n");

    code_function();

    printf("\n[GLOBAL SEGMENT]\n");
    printf("Address of global_initialized   : %p\n",
           (void *)&global_initialized);

    printf("Address of global_uninitialized : %p\n",
           (void *)&global_uninitialized);

    printf("\n[STATIC SEGMENT]\n");
    printf("Address of static_global        : %p\n",
           (void *)&static_global);

    printf("Address of static_local         : %p\n",
           (void *)&static_local);

    printf("\n[HEAP SEGMENT]\n");
    printf("Address of heap_variable        : %p\n",
           (void *)heap_variable);

    printf("\n[STACK SEGMENT]\n");
    printf("Address of stack_variable       : %p\n",
           (void *)&stack_variable);

    printf("\n======================================\n");

    free(heap_variable);
}

void display_memory_maps()
{
    char path[64];

    snprintf(path, sizeof(path), "/proc/%d/maps", getpid());

    printf("\n========== /proc/%d/maps ==========\n", getpid());

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("Unable to open /proc/PID/maps");
        return;
    }

    char line[512];

    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);

    printf("===================================\n");
}

int main()
{
    printf("Linux Process Memory Analysis\n");
    printf("Process ID (PID): %d\n", getpid());

    display_memory_addresses();

    display_memory_maps();

    printf("\nProcess is running for 60 seconds.\n");
    printf("Use another terminal to run:\n");
    printf("pmap %d\n", getpid());
    printf("or:\n");
    printf("cat /proc/%d/maps\n", getpid());

    sleep(60);

    return 0;
}
