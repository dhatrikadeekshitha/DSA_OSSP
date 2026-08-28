# Skill_06 - Command Tokenizer and Parser

## Objective

To split input into tokens, identify delimiters, handle whitespace,
create token structures, validate token streams, debug parsing output,
design parser logic, generate parse trees, validate syntax, detect
errors, handle empty commands, and produce execution structures.

## Features

- Input tokenization
- Whitespace handling
- Delimiter identification
- Token structures
- Token stream validation
- Parser logic
- Parse tree generation
- Syntax validation
- Syntax error detection
- Empty command handling
- Execution structure generation

## Supported Syntax

command arguments

command | command

command < input.txt

command > output.txt

command >> output.txt

## Compilation

gcc -Wall -Wextra -g parser.c -o parser

## Execution

./parser

## Example

ls | grep txt

cat < input.txt

echo hello > output.txt

echo hello >> output.txt
