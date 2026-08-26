# Practical 13 - Producer-Consumer, Semaphores and Deadlock

## Aim

To implement the Producer-Consumer problem using POSIX threads and counting semaphores, evaluate synchronization correctness and throughput for different buffer sizes, demonstrate a deadlock scenario involving multiple threads, identify the four necessary conditions for deadlock, and prevent deadlock using resource ordering.

---

# Part A - Producer-Consumer Problem

The program `producer_consumer.c` implements the Producer-Consumer problem using:

- POSIX threads
- Counting semaphores
- Mutex synchronization
- Circular buffer

## POSIX Functions Used

```text
pthread_create()
pthread_join()
pthread_mutex_lock()
pthread_mutex_unlock()
sem_init()
sem_wait()
sem_post()
sem_destroy()
