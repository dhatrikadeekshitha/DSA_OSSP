# Skill_10 - Variable Expansion and Built-in Dispatch

## Objective

To detect variable references, expand values, handle undefined
variables, update tokens, support nested variable syntax, test
expansion logic, identify built-ins, create a dispatch table, execute
in-process commands, handle invalid commands, maintain shell state,
and test dispatch logic.

## Variable Expansion Features

- Detect $VARIABLE references
- Support ${VARIABLE} syntax
- Expand environment variables
- Create user-defined variables
- Update variable values
- Handle undefined variables
- Expand multiple variables
- Update command tokens after expansion

## Built-in Commands

cd
pwd
echo
set
vars
help
exit

## Examples

echo $USER

echo $HOME

echo ${USER}

set NAME=Student

echo $NAME

set NAME=Developer

echo $NAME

cd /tmp

pwd

vars

## Built-in Dispatch

A dispatch table maps command names to their
corresponding built-in functions.

## In-Process Execution

Built-in commands are executed in the current
process. This allows commands such as cd to
maintain shell state.

## Compilation

gcc -Wall -Wextra -g variable_builtins.c -o variable_builtins

## Execution

./variable_builtins
