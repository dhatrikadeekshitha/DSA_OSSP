# Skill-21 – Signal Interrupt Handling

## Objectives

1. Register signal handlers, configure signal actions, handle interrupts,
   manage signal context, verify stability, and test signal events.

2. Capture SIGINT, forward signals, protect the shell process,
   terminate foreground jobs, update job states, and test interrupt handling.

## Source File

- `signal_interrupt_handler.c`

## Compilation

```bash
gcc -Wall -Wextra signal_interrupt_handler.c -o signal_interrupt_handler
