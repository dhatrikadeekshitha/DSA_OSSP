# Skill_09 - Process Synchronization and PATH Resolution

## Objective

To implement process synchronization and child process monitoring
using waitpid(), retrieve the PATH environment variable, parse search
directories, locate executables, verify permissions, handle missing
commands, and test command resolution.

## Features

- Retrieve PATH using getenv()
- Parse PATH directories
- Search directories for executables
- Verify execute permissions using access()
- Verify regular files using stat()
- Handle missing commands
- Create child processes using fork()
- Execute programs using execv()
- Pass command arguments
- Synchronize parent and child using waitpid()
- Monitor child exit status
- Handle execution errors

## Built-in Commands

path
dirs
resolve <command>
exit

## Example Commands

pwd

ls -l

echo Hello Skill 09

date

sleep 2

resolve ls

resolve abcxyz

## Compilation

gcc -Wall -Wextra -g path_resolver.c -o path_resolver

## Execution

./path_resolver

## Process Flow

Parent
   |
 fork()
   |
 +---------+
 |         |
Parent    Child
 |         |
waitpid() execv()
 |         |
 |       Program
 |         |
 +---<-----+
 |
Continue
