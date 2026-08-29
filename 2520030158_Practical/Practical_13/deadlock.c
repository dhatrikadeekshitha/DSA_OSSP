#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t resource1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resource2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1_function(void *arg)
{
    printf("Thread 1: Locking Resource 1...\n");
    pthread_mutex_lock(&resource1);

    printf("Thread 1: Resource 1 locked.\n");

    sleep(1);

    printf("Thread 1: Waiting for Resource 2...\n");
    pthread_mutex_lock(&resource2);

    printf("Thread 1: Resource 2 locked.\n");

    pthread_mutex_unlock(&resource2);
    pthread_mutex_unlock(&resource1);

    return NULL;
}

void *thread2_function(void *arg)
{
    printf("Thread 2: Locking Resource 2...\n");
    pthread_mutex_lock(&resource2);

    printf("Thread 2: Resource 2 locked.\n");

    sleep(1);

    printf("Thread 2: Waiting for Resource 1...\n");
    pthread_mutex_lock(&resource1);

    printf("Thread 2: Resource 1 locked.\n");

    pthread_mutex_unlock(&resource1);
    pthread_mutex_unlock(&resource2);

    return NULL;
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;

    printf("Deadlock Demonstration\n");
    printf("======================\n\n");

    pthread_create(&thread1, NULL,
                   thread1_function, NULL);

    pthread_create(&thread2, NULL,
                   thread2_function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&resource1);
    pthread_mutex_destroy(&resource2);

    return 0;
}
