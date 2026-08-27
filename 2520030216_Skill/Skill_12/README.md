# Skill_12 - Shell State and Export

## Objective

To retrieve the current directory, display the path, process exit
requests, clean up resources, save shell state, verify correct
behavior, parse export syntax, update environment variables, validate
variable names, handle existing variables, export variables to child
processes, and test functionality.

## Features

### Shell State

- Retrieve current directory using getcwd()
- Display current path
- Maintain previous directory
- Process exit requests
- Save shell state
- Clean up resources
- Handle invalid commands

### Export

- Parse export NAME=VALUE syntax
- Validate environment variable names
- Create environment variables
- Update existing variables
- Use setenv()
- Retrieve variables using getenv()
- Export variables to child processes
- Test inherited environment values

## Built-in Commands

pwd
cd [directory]
cd -
echo TEXT
export NAME=VALUE
export NAME
env
state
testexport
help
exit

## Examples

export NAME=Student

export NAME

export NAME=Developer

export NAME

export SKILL12_TEST=HelloChild

testexport

pwd

cd /tmp

state

cd -

pwd

## Compilation

gcc -Wall -Wextra -g export_shell.c -o export_shell

## Execution

./export_shell

## Child Process Test

The testexport command creates a child process using fork().
The child retrieves the exported variable using getenv().
The parent waits for the child using waitpid().

## Exit and Cleanup

The exit command saves the current shell state and performs
resource cleanup before terminating.
