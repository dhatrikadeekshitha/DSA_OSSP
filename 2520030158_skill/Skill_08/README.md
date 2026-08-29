# Skill_08 - Escape Sequences and Process Launcher

## Objective

To create process escape sequences, handle escaped spaces, escape
special symbols, preserve characters, validate parser output, test
complex inputs, create child processes, execute programs, handle
execution errors, pass arguments, manage the parent process, and test
command launching.

## Features

- Escape sequence processing
- Escaped space handling
- Escaped special symbol handling
- Character preservation
- Command parsing
- Parser output validation
- Child process creation using fork()
- Program execution using execvp()
- Argument passing
- Execution error handling
- Parent process management using waitpid()
- Dynamic argument memory management

## Examples

echo hello

echo hello\ world

echo hello\|world

echo \$HOME

ls -l

pwd

## Compilation

gcc -Wall -Wextra -g process_launcher.c -o process_launcher

## Execution

./process_launcher

## Process Flow

Parent
  |
fork()
  |
+----------------+
|                |
Parent          Child
|                |
waitpid()       execvp()
|                |
Continue        Program
