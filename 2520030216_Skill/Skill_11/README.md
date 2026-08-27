# Skill_11 - Directory Navigation and Built-in Dispatch

## Objective

To change directories, validate paths, update the working directory,
handle errors, maintain the previous directory, test navigation,
identify built-ins, create a dispatch table, execute in-process
commands, handle invalid commands, maintain state, and test dispatch
logic.

## Features

- Change directories using chdir()
- Validate directory paths
- Retrieve current directory using getcwd()
- Maintain previous directory
- Support cd -
- Support cd with no argument
- Handle invalid directory errors
- Built-in command identification
- Built-in dispatch table
- In-process command execution
- Shell state maintenance
- Invalid command handling

## Built-in Commands

cd [directory]
cd -
pwd
echo TEXT
home
state
help
exit

## Examples

cd /tmp

pwd

cd -

pwd

cd /home

state

home

echo Skill_11

## Compilation

gcc -Wall -Wextra -g directory_builtins.c -o directory_builtins

## Execution

./directory_builtins

## Directory Navigation Flow

Input
  |
  v
Parse Command
  |
  v
Identify Built-in
  |
  v
Dispatch Table
  |
  v
builtin_cd()
  |
  v
Validate Path
  |
  v
chdir()
  |
  v
Update Current Directory
  |
  v
Store Previous Directory
