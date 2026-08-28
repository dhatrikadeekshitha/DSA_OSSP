# Skill_15 - Input and Output Redirection

## Objective

To parse input and output redirection, open files, redirect standard
input and output streams, handle missing files and permission errors,
restore streams, verify output data, and test file redirection.

## Features

- Parse input redirection using `<`
- Open input files using `open()`
- Redirect stdin using `dup2()`
- Handle missing input files
- Restore stdin after redirection
- Parse output redirection using `>`
- Create output files using `open()`
- Redirect stdout using `dup2()`
- Handle output permission errors
- Restore stdout after redirection
- Verify output file using `stat()`
- Test input and output redirection

## System Calls Used

- open()
- close()
- dup()
- dup2()
- read through stdin
- write through stdout
- stat()
- getcwd()

## Compilation

gcc -Wall -Wextra -Werror -std=c11 -g redirection.c -o redirection

## Execution

./redirection

## Input Redirection Test

cat < input.txt

## Output Redirection Test

echo hello > output.txt

## Missing File Test

cat < missing.txt

## Permission Test

echo hello > protected/output.txt

## Stream Restoration

After each redirection operation, stdin/stdout is restored so that
subsequent terminal input and output continue normally.
