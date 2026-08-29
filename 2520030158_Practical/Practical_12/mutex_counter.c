#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define ITERATIONS 1000000

long counter = 0;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *increment_counter(void *arg)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        pthread_mutex_lock(&counter_mutex);

        counter++;

        pthread_mutex_unlock(&counter_mutex);
    }

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];

    printf("Mutex Synchronization Demonstration\n");
    printf("===================================\n");

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
        printf("Result: Correct - synchronization successful\n");
    }
    else
    {
        printf("Result: Incorrect\n");
    }

    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
