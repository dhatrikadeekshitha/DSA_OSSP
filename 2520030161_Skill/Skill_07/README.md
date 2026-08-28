# Skill_07 - Single and Double Quote Parser

## Objective

To apply single quotes, preserve literal content, ignore variable
expansion, store quoted strings, validate parsing results, test edge
cases, apply double quotes, preserve spaces, allow variable expansion,
parse nested tokens, validate outputs, and test quoted commands.

## Features

- Single quote parsing
- Literal content preservation
- Variable expansion disabled inside single quotes
- Double quote parsing
- Space preservation
- Variable expansion inside double quotes
- Multiple quoted tokens
- Token validation
- Unterminated quote detection
- Empty command handling
- Parsing output validation

## Examples

echo 'hello world'

echo '$USER'

echo "hello world"

echo "$USER"

echo 'User: $USER' "Home: $HOME"

## Compilation

gcc -Wall -Wextra -g quotes_parser.c -o quotes_parser

## Execution

./quotes_parser
