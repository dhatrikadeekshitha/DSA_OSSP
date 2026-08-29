# Skill_05 - Command History and Dynamic Memory

## Objective

To apply escape sequences, store command history, navigate previous
and next commands, update the input buffer, test command recall,
allocate buffers dynamically, resize arrays, prevent buffer overflow,
manage linked lists, release memory correctly, and verify memory
management using Valgrind.

## Features

- Escape sequence handling
- Up arrow command navigation
- Down arrow command navigation
- Command history
- Dynamic input buffer
- Automatic buffer resizing using realloc()
- Buffer overflow prevention
- Linked list for command history
- Proper memory deallocation
- Valgrind memory verification

## Commands

help
hello
status
history
exit

## Compilation

gcc -Wall -Wextra -g command_history.c -o command_history

## Execution

./command_history

## Valgrind

valgrind --leak-check=full --show-leak-kinds=all ./command_history
