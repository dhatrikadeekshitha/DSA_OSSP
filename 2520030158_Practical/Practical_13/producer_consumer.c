#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>

#define ITEMS 100000

int *buffer;
int buffer_size;

int in = 0;
int out = 0;

sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t buffer_mutex;

double get_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void *producer(void *arg)
{
    for (int i = 0; i < ITEMS; i++)
    {
        sem_wait(&empty_slots);

        pthread_mutex_lock(&buffer_mutex);

        buffer[in] = i;
        in = (in + 1) % buffer_size;

        pthread_mutex_unlock(&buffer_mutex);

        sem_post(&full_slots);
    }

    return NULL;
}

void *consumer(void *arg)
{
    long sum = 0;

    for (int i = 0; i < ITEMS; i++)
    {
        sem_wait(&full_slots);

        pthread_mutex_lock(&buffer_mutex);

        int item = buffer[out];
        out = (out + 1) % buffer_size;

        pthread_mutex_unlock(&buffer_mutex);

        sem_post(&empty_slots);

        sum += item;
    }

    printf("Consumer processed %d items. Checksum: %ld\n",
           ITEMS, sum);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <buffer_size>\n", argv[0]);
        printf("Example: %s 10\n", argv[0]);
        return 1;
    }

    buffer_size = atoi(argv[1]);

    if (buffer_size <= 0)
    {
        printf("Buffer size must be greater than 0.\n");
        return 1;
    }

    buffer = malloc(buffer_size * sizeof(int));

    if (buffer == NULL)
    {
        perror("malloc");
        return 1;
    }

    pthread_t producer_thread;
    pthread_t consumer_thread;

    sem_init(&empty_slots, 0, buffer_size);
    sem_init(&full_slots, 0, 0);

    pthread_mutex_init(&buffer_mutex, NULL);

    printf("Producer-Consumer Demonstration\n");
    printf("================================\n");
    printf("Buffer size: %d\n", buffer_size);
    printf("Items: %d\n\n", ITEMS);

    double start = get_time();

    pthread_create(&producer_thread, NULL,
                   producer, NULL);

    pthread_create(&consumer_thread, NULL,
                   consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    double end = get_time();

    double elapsed = end - start;
    double throughput = ITEMS / elapsed;

    printf("\nExecution time: %.6f seconds\n", elapsed);
    printf("Throughput: %.2f items/second\n", throughput);

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);

    pthread_mutex_destroy(&buffer_mutex);

    free(buffer);

    return 0;
}
