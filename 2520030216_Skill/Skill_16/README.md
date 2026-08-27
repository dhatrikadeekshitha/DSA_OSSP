# Skill_16 - Append and stderr Redirection

## Objective

To parse the append operator, open files in append mode, redirect
streams, preserve existing data, validate appended data, parse stderr
redirection, redirect error streams, capture error output, verify file
contents, handle failures, and test error cases.

## Features

- Parse append operator `>>`
- Open files using `O_APPEND`
- Create files when required using `O_CREAT`
- Redirect stdout using `dup2()`
- Preserve existing file contents
- Validate appended data
- Restore stdout
- Parse stderr redirection `2>`
- Redirect stderr using `dup2()`
- Capture error output
- Verify error file contents
- Handle file opening failures
- Handle command failures
- Restore stderr
- Test missing-file errors

## System Calls and Functions Used

- open()
- close()
- dup()
- dup2()
- stat()
- system()
- wait-related status macros
- fopen()
- fgets()
- fprintf()

## Compilation

gcc -Wall -Wextra -Werror -std=c11 -g append_stderr.c -o append_stderr

## Execution

./append_stderr

## Append Redirection

echo first >> append.txt

echo second >> append.txt

echo third >> append.txt

## stderr Redirection

ls missing_file 2> error.txt

## Verification

cat append.txt

cat error.txt

## Expected Behavior

The `>>` operator preserves existing data and appends new output to
the end of the file.

The `2>` operator redirects standard error to a file, allowing error
messages to be captured and verified.
