# Case Study 9 – Process Termination

## Aim

To implement process termination using the `exit()` system call in Linux.

## Description

The program creates a child process using `fork()`.
The child process terminates using `exit(0)`.
The parent process waits for the child using `wait()` and then terminates.

## Compilation

```bash
gcc process_termination.c -o process_termination
