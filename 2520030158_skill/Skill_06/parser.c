#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 256
#define MAX_TOKENS 50

/* =========================================
   TOKEN DEFINITIONS
   ========================================= */

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_END
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

/* =========================================
   TOKEN TYPE NAME
   ========================================= */

const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_REDIRECT_IN:
            return "REDIRECT_IN";

        case TOKEN_REDIRECT_OUT:
            return "REDIRECT_OUT";

        case TOKEN_REDIRECT_APPEND:
            return "REDIRECT_APPEND";

        case TOKEN_END:
            return "END";

        default:
            return "UNKNOWN";
    }
}

/* =========================================
   CREATE TOKEN
   ========================================= */

Token create_token(TokenType type, const char *value)
{
    Token token;

    token.type = type;

    token.value = malloc(strlen(value) + 1);

    if (token.value == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(token.value, value);

    return token;
}

/* =========================================
   FREE TOKENS
   ========================================= */

void free_tokens(Token tokens[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free(tokens[i].value);
    }
}

/* =========================================
   ADD TOKEN
   ========================================= */

void add_token(Token tokens[],
               int *count,
               TokenType type,
               const char *value)
{
    if (*count >= MAX_TOKENS - 1)
    {
        printf("Error: Too many tokens.\n");
        return;
    }

    tokens[*count] = create_token(type, value);
    (*count)++;
}

/* =========================================
   TOKENIZER
   ========================================= */

int tokenize(const char *input, Token tokens[])
{
    int i = 0;
    int count = 0;

    while (input[i] != '\0')
    {
        /* Skip whitespace */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe delimiter */
        if (input[i] == '|')
        {
            add_token(tokens, &count,
                      TOKEN_PIPE, "|");

            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            add_token(tokens, &count,
                      TOKEN_REDIRECT_IN, "<");

            i++;
            continue;
        }

        /* Output redirection */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                add_token(tokens, &count,
                          TOKEN_REDIRECT_APPEND, ">>");

                i += 2;
            }
            else
            {
                add_token(tokens, &count,
                          TOKEN_REDIRECT_OUT, ">");

                i++;
            }

            continue;
        }

        /* Word token */
        char word[MAX_INPUT];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>')
        {
            if (j < MAX_INPUT - 1)
            {
                word[j++] = input[i];
            }

            i++;
        }

        word[j] = '\0';

        if (j > 0)
        {
            add_token(tokens, &count,
                      TOKEN_WORD, word);
        }
    }

    add_token(tokens, &count,
              TOKEN_END, "END");

    return count;
}

/* =========================================
   DISPLAY TOKENS
   ========================================= */

void display_tokens(Token tokens[], int count)
{
    printf("\n========== TOKEN STREAM ==========\n");

    for (int i = 0; i < count; i++)
    {
        printf("[%d] %-16s : %s\n",
               i,
               token_type_name(tokens[i].type),
               tokens[i].value);
    }

    printf("==================================\n");
}

/* =========================================
   PARSE TREE
   ========================================= */

typedef struct ParseNode
{
    char *type;
    char *value;

    struct ParseNode *left;
    struct ParseNode *right;
} ParseNode;

/* =========================================
   CREATE PARSE NODE
   ========================================= */

ParseNode *create_parse_node(const char *type,
                              const char *value)
{
    ParseNode *node =
        malloc(sizeof(ParseNode));

    if (node == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->type = malloc(strlen(type) + 1);
    node->value = malloc(strlen(value) + 1);

    if (node->type == NULL ||
        node->value == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(node->type, type);
    strcpy(node->value, value);

    node->left = NULL;
    node->right = NULL;

    return node;
}

/* =========================================
   FREE PARSE TREE
   ========================================= */

void free_parse_tree(ParseNode *root)
{
    if (root == NULL)
        return;

    free_parse_tree(root->left);
    free_parse_tree(root->right);

    free(root->type);
    free(root->value);
    free(root);
}

/* =========================================
   DISPLAY PARSE TREE
   ========================================= */

void print_parse_tree(ParseNode *root,
                       int level)
{
    if (root == NULL)
        return;

    for (int i = 0; i < level; i++)
    {
        printf("  ");
    }

    printf("|-- %s : %s\n",
           root->type,
           root->value);

    print_parse_tree(root->left,
                     level + 1);

    print_parse_tree(root->right,
                     level + 1);
}

/* =========================================
   SYNTAX VALIDATION
   ========================================= */

int validate_syntax(Token tokens[],
                    int count)
{
    if (count <= 1)
    {
        printf("\nSyntax Error: Empty command.\n");
        return 0;
    }

    /* Command must begin with a WORD */
    if (tokens[0].type != TOKEN_WORD)
    {
        printf("\nSyntax Error: Command must begin with a word.\n");
        return 0;
    }

    for (int i = 0; i < count - 1; i++)
    {
        /* Pipe cannot appear at beginning or end */
        if (tokens[i].type == TOKEN_PIPE)
        {
            if (i == 0 ||
                tokens[i + 1].type == TOKEN_PIPE)
            {
                printf("\nSyntax Error: Invalid pipe placement.\n");
                return 0;
            }
        }

        /* Pipe must be followed by a command */
        if (tokens[i].type == TOKEN_PIPE &&
            tokens[i + 1].type != TOKEN_WORD)
        {
            printf("\nSyntax Error: Pipe must be followed by a command.\n");
            return 0;
        }

        /* Redirection must be followed by a filename */
        if (tokens[i].type == TOKEN_REDIRECT_IN ||
            tokens[i].type == TOKEN_REDIRECT_OUT ||
            tokens[i].type == TOKEN_REDIRECT_APPEND)
        {
            if (tokens[i + 1].type != TOKEN_WORD)
            {
                printf("\nSyntax Error: Redirection requires a filename.\n");
                return 0;
            }
        }
    }

    /* Command cannot end with a pipe */
    if (tokens[count - 2].type == TOKEN_PIPE)
    {
        printf("\nSyntax Error: Command cannot end with a pipe.\n");
        return 0;
    }

    /* Redirection cannot be the final token */
    if (tokens[count - 2].type == TOKEN_REDIRECT_IN ||
        tokens[count - 2].type == TOKEN_REDIRECT_OUT ||
        tokens[count - 2].type == TOKEN_REDIRECT_APPEND)
    {
        printf("\nSyntax Error: Redirection requires a filename.\n");
        return 0;
    }

    printf("\nSyntax validation successful.\n");

    return 1;
}

/* =========================================
   BUILD PARSE TREE
   ========================================= */

ParseNode *build_parse_tree(Token tokens[],
                            int count)
{
    ParseNode *root =
        create_parse_node("COMMAND",
                          "COMMAND");

    ParseNode *current = root;

    int i = 0;

    while (i < count - 1)
    {
        if (tokens[i].type == TOKEN_WORD)
        {
            ParseNode *word =
                create_parse_node("WORD",
                                  tokens[i].value);

            if (current->left == NULL)
            {
                current->left = word;
            }
            else
            {
                current->right = word;
            }

            i++;
        }
        else if (tokens[i].type == TOKEN_PIPE)
        {
            ParseNode *pipe =
                create_parse_node("PIPE",
                                  "|");

            pipe->left = current;

            ParseNode *next_command =
                create_parse_node("COMMAND",
                                  "COMMAND");

            pipe->right = next_command;

            root = pipe;
            current = next_command;

            i++;
        }
        else if (tokens[i].type == TOKEN_REDIRECT_IN)
        {
            ParseNode *redirect =
                create_parse_node("REDIRECT_IN",
                                  "<");

            current->right = redirect;

            if (i + 1 < count)
            {
                ParseNode *file =
                    create_parse_node("FILE",
                                      tokens[i + 1].value);

                redirect->left = file;
            }

            i += 2;
        }
        else if (tokens[i].type == TOKEN_REDIRECT_OUT)
        {
            ParseNode *redirect =
                create_parse_node("REDIRECT_OUT",
                                  ">");

            current->right = redirect;

            if (i + 1 < count)
            {
                ParseNode *file =
                    create_parse_node("FILE",
                                      tokens[i + 1].value);

                redirect->left = file;
            }

            i += 2;
        }
        else if (tokens[i].type == TOKEN_REDIRECT_APPEND)
        {
            ParseNode *redirect =
                create_parse_node("REDIRECT_APPEND",
                                  ">>");

            current->right = redirect;

            if (i + 1 < count)
            {
                ParseNode *file =
                    create_parse_node("FILE",
                                      tokens[i + 1].value);

                redirect->left = file;
            }

            i += 2;
        }
        else
        {
            i++;
        }
    }

    return root;
}

/* =========================================
   EXECUTION STRUCTURE
   ========================================= */

void display_execution_structure(Token tokens[],
                                 int count)
{
    printf("\n======= EXECUTION STRUCTURE =======\n");

    printf("Command : ");

    for (int i = 0; i < count - 1; i++)
    {
        if (tokens[i].type == TOKEN_WORD)
        {
            printf("%s ", tokens[i].value);
        }
    }

    printf("\n");

    for (int i = 0; i < count - 1; i++)
    {
        if (tokens[i].type == TOKEN_REDIRECT_IN)
        {
            printf("Input File  : %s\n",
                   tokens[i + 1].value);
        }

        if (tokens[i].type == TOKEN_REDIRECT_OUT)
        {
            printf("Output File : %s\n",
                   tokens[i + 1].value);
        }

        if (tokens[i].type == TOKEN_REDIRECT_APPEND)
        {
            printf("Append File : %s\n",
                   tokens[i + 1].value);
        }
    }

    printf("Pipelines : ");

    int pipes = 0;

    for (int i = 0; i < count; i++)
    {
        if (tokens[i].type == TOKEN_PIPE)
        {
            pipes++;
        }
    }

    printf("%d\n", pipes);

    printf("====================================\n");
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[MAX_INPUT];

    printf("========================================\n");
    printf("        SKILL_06 COMMAND PARSER\n");
    printf("========================================\n");

    printf("Supported syntax:\n");
    printf("  command arguments\n");
    printf("  command | command\n");
    printf("  command < input.txt\n");
    printf("  command > output.txt\n");
    printf("  command >> output.txt\n");
    printf("\nType 'exit' to quit.\n");

    while (1)
    {
        printf("\nskill06> ");
        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /* Handle empty commands */
        if (strlen(input) == 0)
        {
            printf("Empty command. Nothing to parse.\n");
            continue;
        }

        /* Exit */
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting Skill_06...\n");
            break;
        }

        Token tokens[MAX_TOKENS];

        int token_count =
            tokenize(input, tokens);

        /* Display tokenizer output */
        display_tokens(tokens,
                       token_count);

        /* Validate syntax */
        if (validate_syntax(tokens,
                            token_count))
        {
            /* Build parse tree */
            ParseNode *tree =
                build_parse_tree(tokens,
                                 token_count);

            printf("\n=========== PARSE TREE ===========\n");

            print_parse_tree(tree, 0);

            printf("==================================\n");

            /* Execution structure */
            display_execution_structure(
                tokens,
                token_count);

            free_parse_tree(tree);
        }

        free_tokens(tokens,
                    token_count);
    }

    return 0;
}
