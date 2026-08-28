# Skill-22 – Process Suspension and Terminal Control

## Objectives

1. Capture SIGTSTP, suspend processes, update the job table,
   preserve process state, resume processes later, and test the
   suspension workflow.

2. Create process groups, assign group IDs, transfer terminal control,
   restore terminal ownership, coordinate signals, and validate behavior.

## Source File

- `process_suspension_terminal.c`

## Compilation

```bash
gcc -Wall -Wextra process_suspension_terminal.c -o process_suspension_terminal
