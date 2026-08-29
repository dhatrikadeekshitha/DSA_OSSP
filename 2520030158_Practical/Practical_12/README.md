# Practical 12 - Threads, Race Conditions and Mutex Synchronization

## Aim

To develop a multithreaded counter application using POSIX threads and demonstrate race conditions when multiple threads update a shared variable concurrently. The program is then modified using mutex locks to provide synchronization and the results are compared.

---

## Part A - Race Condition

The program `race_condition.c` creates multiple threads using:

```c
pthread_create()
