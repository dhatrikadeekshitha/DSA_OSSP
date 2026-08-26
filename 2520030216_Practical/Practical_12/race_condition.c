#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define ITERATIONS 1000000

long counter = 0;

void *increment_counter(void *arg)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        counter++;
    }

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];

    printf("Race Condition Demonstration\n");
    printf("============================\n");

    printf("Threads: %d\n", NUM_THREADS);
    printf("Iterations per thread: %d\n", ITERATIONS);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL,
                       increment_counter, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("\nExpected counter value: %ld\n",
           (long)NUM_THREADS * ITERATIONS);

    printf("Actual counter value:   %ld\n", counter);

    if (counter == (long)NUM_THREADS * ITERATIONS)
    {
        printf("Result: Correct\n");
    }
    else
    {
        printf("Result: Race condition detected\n");
    }

    return 0;
}
