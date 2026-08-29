#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Dynamic Memory Allocation Demonstration\n");
    printf("=======================================\n");

    // malloc()
    int *arr = malloc(5 * sizeof(int));

    if (arr == NULL)
    {
        printf("malloc failed\n");
        return 1;
    }

    for (int i = 0; i < 5; i++)
    {
        arr[i] = (i + 1) * 10;
    }

    printf("\nAfter malloc():\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // calloc()
    int *zero_arr = calloc(5, sizeof(int));

    if (zero_arr == NULL)
    {
        printf("calloc failed\n");
        free(arr);
        return 1;
    }

    printf("\nAfter calloc():\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", zero_arr[i]);
    }
    printf("\n");

    // realloc()
    arr = realloc(arr, 10 * sizeof(int));

    if (arr == NULL)
    {
        printf("realloc failed\n");
        free(zero_arr);
        return 1;
    }

    for (int i = 5; i < 10; i++)
    {
        arr[i] = (i + 1) * 10;
    }

    printf("\nAfter realloc():\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // free()
    free(arr);
    free(zero_arr);

    printf("\nMemory successfully released using free().\n");

    return 0;
}
