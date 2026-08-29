# Skill_14 - Multiple Pipes and Stream Redirection

## Objective

To create pipes, redirect streams, execute connected commands,
close descriptors, synchronize processes, support multiple pipes,
manage descriptor arrays, launch multiple processes, coordinate
execution, clean up resources, and test long pipelines.

## Features

- Create pipes using pipe()
- Redirect streams using dup2()
- Execute commands using execvp()
- Create multiple processes using fork()
- Support multiple pipeline stages
- Maintain pipe descriptor arrays
- Close unused descriptors
- Synchronize processes using waitpid()
- Support input redirection <
- Support output redirection >
- Support append redirection >>
- Support error redirection 2>
- Support error append redirection 2>>
- Cleanup allocated resources
- Test long pipelines

## Compilation

gcc -Wall -Wextra -Werror -std=c11 -g pipeline_shell.c -o pipeline_shell

## Execution

./pipeline_shell

## Test Cases

echo hello | tr a-z A-Z

seq 1 20 | cat | cat | cat | wc -l

seq 1 100 | cat | cat | cat | cat | cat | cat | wc -l

echo hello | tr a-z A-Z > output.txt

cat < output.txt

echo world >> output.txt

ls missing_file 2> error.txt
