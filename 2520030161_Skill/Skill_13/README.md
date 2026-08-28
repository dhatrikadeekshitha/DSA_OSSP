# Skill_13 - Command History and Pipeline Execution

## Objective

To store commands, maintain a history buffer, handle capacity limits,
retrieve entries, display history, validate consistency, create
pipeline structures, store commands, maintain execution order, connect
processes, validate pipeline layout, and test pipeline configurations.

## Part 1 - Command History

### Features

- Store executed commands
- Maintain a fixed-size history buffer
- Maximum history capacity of 10 commands
- Remove oldest command when capacity is reached
- Retrieve history entries
- Display complete history
- Validate history consistency
- Release history memory correctly

### Commands

history
history N
validate

### Example

history

history 2

validate

## Part 2 - Pipelines

### Features

- Create pipeline structures
- Store individual pipeline commands
- Store command arguments
- Maintain command execution order
- Validate pipeline layout
- Create pipes using pipe()
- Create child processes using fork()
- Connect processes using dup2()
- Execute commands using execvp()
- Wait for child processes using waitpid()
- Handle execution errors

### Examples

ls | wc -l

echo hello | tr a-z A-Z

ls | grep Skill | wc -l

printf hello | wc -c

## Pipeline Structure

Stage 1 -> Stage 2 -> Stage 3

Each connection between stages uses a pipe.

## Pipeline Execution

For each command:

1. Parse command
2. Store command in pipeline structure
3. Validate pipeline
4. Create required pipes
5. Create child processes
6. Connect stdin/stdout
7. Execute commands
8. Parent waits for children
9. Release resources

## Compilation

gcc -Wall -Wextra -g history_pipeline.c -o history_pipeline

## Execution

./history_pipeline

## Memory Verification

valgrind --leak-check=full ./history_pipeline

## Cleanup

The program releases all dynamically allocated history
and pipeline resources before exiting.
