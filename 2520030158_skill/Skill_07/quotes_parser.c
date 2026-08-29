#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 512
#define MAX_TOKENS 50

typedef enum
{
    TOKEN_WORD,
    TOKEN_SINGLE_QUOTED,
    TOKEN_DOUBLE_QUOTED,
    TOKEN_VARIABLE,
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

        case TOKEN_SINGLE_QUOTED:
            return "SINGLE_QUOTED";

        case TOKEN_DOUBLE_QUOTED:
            return "DOUBLE_QUOTED";

        case TOKEN_VARIABLE:
            return "VARIABLE";

        case TOKEN_END:
            return "END";

        default:
            return "UNKNOWN";
    }
}

/* =========================================
   DUPLICATE STRING
   ========================================= */

char *duplicate_string(const char *source)
{
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, source);

    return copy;
}

/* =========================================
   CREATE TOKEN
   ========================================= */

Token create_token(TokenType type, const char *value)
{
    Token token;

    token.type = type;
    token.value = duplicate_string(value);

    return token;
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
   VARIABLE EXPANSION
   ========================================= */

char *expand_variables(const char *input)
{
    char result[MAX_INPUT];
    int i = 0;
    int j = 0;

    while (input[i] != '\0' &&
           j < MAX_INPUT - 1)
    {
        if (input[i] == '$')
        {
            char variable[100];
            int k = 0;

            i++;

            /* ${VARIABLE} format */
            if (input[i] == '{')
            {
                i++;

                while (input[i] != '\0' &&
                       input[i] != '}' &&
                       k < 99)
                {
                    variable[k++] = input[i++];
                }

                if (input[i] == '}')
                    i++;
            }
            else
            {
                /* $VARIABLE format */
                while (input[i] != '\0' &&
                       (isalnum((unsigned char)input[i]) ||
                        input[i] == '_') &&
                       k < 99)
                {
                    variable[k++] = input[i++];
                }
            }

            variable[k] = '\0';

            const char *value =
                getenv(variable);

            if (value != NULL)
            {
                for (int x = 0;
                     value[x] != '\0' &&
                     j < MAX_INPUT - 1;
                     x++)
                {
                    result[j++] = value[x];
                }
            }
        }
        else
        {
            result[j++] = input[i++];
        }
    }

    result[j] = '\0';

    return duplicate_string(result);
}

/* =========================================
   PARSE INPUT
   ========================================= */

int parse_input(const char *input,
                Token tokens[])
{
    int i = 0;
    int count = 0;

    while (input[i] != '\0')
    {
        /* Ignore whitespace outside quotes */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* -----------------------------
           SINGLE QUOTES
           ----------------------------- */

        if (input[i] == '\'')
        {
            char buffer[MAX_INPUT];
            int j = 0;

            i++;

            while (input[i] != '\0' &&
                   input[i] != '\'')
            {
                if (j < MAX_INPUT - 1)
                {
                    buffer[j++] = input[i];
                }

                i++;
            }

            buffer[j] = '\0';

            if (input[i] != '\'')
            {
                printf("\nSyntax Error: Unterminated single quote.\n");
                return -1;
            }

            i++;

            /*
             * Single quoted strings are stored
             * exactly as written.
             *
             * No variable expansion is performed.
             */

            add_token(tokens,
                      &count,
                      TOKEN_SINGLE_QUOTED,
                      buffer);

            continue;
        }

        /* -----------------------------
           DOUBLE QUOTES
           ----------------------------- */

        if (input[i] == '"')
        {
            char buffer[MAX_INPUT];
            int j = 0;

            i++;

            while (input[i] != '\0' &&
                   input[i] != '"')
            {
                if (input[i] == '\\' &&
                    input[i + 1] != '\0')
                {
                    i++;

                    if (j < MAX_INPUT - 1)
                    {
                        buffer[j++] = input[i];
                    }

                    i++;
                    continue;
                }

                if (j < MAX_INPUT - 1)
                {
                    buffer[j++] = input[i];
                }

                i++;
            }

            buffer[j] = '\0';

            if (input[i] != '"')
            {
                printf("\nSyntax Error: Unterminated double quote.\n");
                return -1;
            }

            i++;

            /*
             * Double quoted strings preserve
             * spaces and allow variable expansion.
             */

            char *expanded =
                expand_variables(buffer);

            add_token(tokens,
                      &count,
                      TOKEN_DOUBLE_QUOTED,
                      expanded);

            free(expanded);

            continue;
        }

        /* -----------------------------
           NORMAL WORD / VARIABLE
           ----------------------------- */

        char buffer[MAX_INPUT];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '\'' &&
               input[i] != '"')
        {
            if (j < MAX_INPUT - 1)
            {
                buffer[j++] = input[i];
            }

            i++;
        }

        buffer[j] = '\0';

        if (j > 0)
        {
            if (buffer[0] == '$')
            {
                char *expanded =
                    expand_variables(buffer);

                add_token(tokens,
                          &count,
                          TOKEN_VARIABLE,
                          expanded);

                free(expanded);
            }
            else
            {
                add_token(tokens,
                          &count,
                          TOKEN_WORD,
                          buffer);
            }
        }
    }

    add_token(tokens,
              &count,
              TOKEN_END,
              "END");

    return count;
}

/* =========================================
   DISPLAY TOKENS
   ========================================= */

void display_tokens(Token tokens[],
                    int count)
{
    printf("\n========== PARSED TOKENS ==========\n");

    for (int i = 0; i < count; i++)
    {
        printf("[%d] %-18s : \"%s\"\n",
               i,
               token_type_name(tokens[i].type),
               tokens[i].value);
    }

    printf("===================================\n");
}

/* =========================================
   VALIDATE TOKENS
   ========================================= */

int validate_tokens(Token tokens[],
                    int count)
{
    if (count <= 1)
    {
        printf("\nValidation Error: No tokens found.\n");
        return 0;
    }

    for (int i = 0; i < count - 1; i++)
    {
        if (tokens[i].value == NULL ||
            strlen(tokens[i].value) == 0)
        {
            printf("\nValidation Error: Empty token detected.\n");
            return 0;
        }
    }

    printf("\nToken validation successful.\n");

    return 1;
}

/* =========================================
   DISPLAY QUOTING RULES
   ========================================= */

void display_quote_analysis(Token tokens[],
                            int count)
{
    printf("\n========== QUOTE ANALYSIS ==========\n");

    for (int i = 0; i < count - 1; i++)
    {
        if (tokens[i].type == TOKEN_SINGLE_QUOTED)
        {
            printf("Token %d: Single quoted\n",
                   i);

            printf("  Content: %s\n",
                   tokens[i].value);

            printf("  Variable expansion: DISABLED\n");
            printf("  Spaces preserved: YES\n");
        }
        else if (tokens[i].type == TOKEN_DOUBLE_QUOTED)
        {
            printf("Token %d: Double quoted\n",
                   i);

            printf("  Content: %s\n",
                   tokens[i].value);

            printf("  Variable expansion: ENABLED\n");
            printf("  Spaces preserved: YES\n");
        }
    }

    printf("====================================\n");
}

/* =========================================
   DISPLAY FINAL OUTPUT
   ========================================= */

void display_execution_output(Token tokens[],
                              int count)
{
    printf("\n========= EXECUTION OUTPUT =========\n");

    for (int i = 0; i < count - 1; i++)
    {
        printf("%s ",
               tokens[i].value);
    }

    printf("\n====================================\n");
}

/* =========================================
   MAIN
   ========================================= */

int main()
{
    char input[MAX_INPUT];

    printf("========================================\n");
    printf("       SKILL_07 QUOTED STRING PARSER\n");
    printf("========================================\n");

    printf("\nFeatures:\n");
    printf("- Single quote handling\n");
    printf("- Literal content preservation\n");
    printf("- Variable expansion control\n");
    printf("- Double quote handling\n");
    printf("- Space preservation\n");
    printf("- Nested token parsing\n");
    printf("- Quote validation\n");

    printf("\nEnvironment variable example:\n");
    printf("$USER = %s\n", getenv("USER"));

    printf("\nType 'exit' to quit.\n");

    while (1)
    {
        printf("\nskill07> ");
        fflush(stdout);

        if (fgets(input,
                  sizeof(input),
                  stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        /* Empty command */
        if (strlen(input) == 0)
        {
            printf("Empty command. Nothing to parse.\n");
            continue;
        }

        /* Exit */
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting Skill_07...\n");
            break;
        }

        Token tokens[MAX_TOKENS];

        int token_count =
            parse_input(input, tokens);

        if (token_count == -1)
        {
            continue;
        }

        display_tokens(tokens,
                       token_count);

        if (validate_tokens(tokens,
                            token_count))
        {
            display_quote_analysis(tokens,
                                   token_count);

            display_execution_output(tokens,
                                     token_count);
        }

        free_tokens(tokens,
                     token_count);
    }

    return 0;
}
