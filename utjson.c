#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum json_type {
    JSON_OBJECT  = 0,
    JSON_ARRAY   = 1,
    JSON_BOOLEAN = 2,
    JSON_NUM_INT = 3,
    JSON_NUM_DBL = 4,
    JSON_STRING  = 5,
} json_type;

typedef struct json {
    int   type;
    int   size;
    void *data;
    struct json *next;
} JSON;

typedef struct kv {
    char *key;
    void *val;
} KV;

JSON *parse_boolean(char *buf);
JSON *parse_num_int(char *buf);
JSON *parse_num_dbl(char *buf);
JSON *parse_string(char *buf);
JSON *parse_array(char *buf);
int   prepend(JSON **head, JSON *item);
void  reserve(JSON **head);
int   list_length(JSON *head);
char *tokenizer(char *buf_in, char *token);
int   is_whitespace(char c);
char *rm_leading_whitespaces(char *buf);
int   is_structural(char c);
JSON *which_type(char *token);

int main(int argc, char **argv)
{
    char *file = argv[1];
    FILE *fp   = fopen(file, "r");
    int   i;
    int   j = 0;
    if (fp == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    printf("Size of the file %d\n", size);
    fseek(fp, 0L, SEEK_SET);

    char buf[size];
    int nitems = fread(buf, sizeof(buf), 1, fp);
    if (nitems != 1)
    {
        printf("Failed to read\n");
        return -1;
    }

    for (i = 0; i < size; i++)
        printf("%c", buf[i]);

    // Parse num int
    // JSON *num_int = parse_num_int(buf);
    // printf("### Type %d Size %d Data %d\n", num_int->type, num_int->size, *(int *)num_int->data);

    // Parse tokens
    char  token[8192] = { '\0' };
    char *p = buf;
    JSON *part;

    while ((p = tokenizer(p, token)) != NULL)
    {
        printf("%s\n", token);
        if ((part = which_type(token)) != NULL)
        {
            printf("### Type %d Size %d\n", part->type, part->size);
        }
    }

    // Push and pop structural tokens
    char stack[128] = { '\0' };
    for (i = 0; i < size; i++)
    {
        if ((stack[j - 1] == '"' && buf[i] == '"') ||
            (stack[j - 1] == '{' && buf[i] == '}') ||
            (stack[j - 1] == '[' && buf[i] == ']'))
        {
            j--;
            stack[j] = '\0';
            printf("----%d\t%s\n", i, stack);
        }
        else if (buf[i] == '{' || buf[i] == '"' || buf[i] == '[')
        {
            stack[j] = buf[i];
            j++;
            printf("++++%d\t%s\n", i, stack);
        }
    }

    fclose(fp);
    return 0;
}

JSON *parse_boolean(char *buf)
{
    JSON *boolean = malloc(sizeof(JSON));

    boolean->type = JSON_BOOLEAN;
    boolean->size = 1;
    boolean->data = malloc(1);
    if (strcmp(buf, "true") == 0)
        *(char *)boolean->data = 1;
    else if (strcmp(buf, "false") == 0)
        *(char *)boolean->data = 0;
    boolean->next = NULL;

    return boolean;
}

JSON *parse_num_int(char *buf)
{
    JSON *num_int = malloc(sizeof(JSON));

    num_int->type = JSON_NUM_INT;
    num_int->size = sizeof(int);
    num_int->data = malloc(sizeof(int));
    *(int *)num_int->data = atoi(buf);
    num_int->next = NULL;

    return num_int;
}

JSON *parse_num_dbl(char *buf)
{
    JSON *num_dbl = malloc(sizeof(JSON));

    num_dbl->type = JSON_NUM_DBL;
    num_dbl->size = sizeof(double);
    num_dbl->data = malloc(sizeof(double));
    *(double *)num_dbl->data = atof(buf);
    num_dbl->next = NULL;

    return num_dbl;
}

JSON *parse_string(char *str)
{
    JSON *string  = malloc(sizeof(JSON));

    string->type = JSON_STRING;
    string->size = strlen(str);
    string->data = str;
    string->next = NULL;

    return string;
}

// JSON **parse_array(char *buf)
// {
//     JSON **aray = malloc(sizeof(JSON *));
//
//     array->type = JSON_ARRAY;
//     array->size =
//
//     return array;
// }

int prepend(JSON **head, JSON *item)
{
    if (item == NULL)
        return -1;

    item->next = *head;
    *head = item;

    return 0;
}

void reserve(JSON **head)
{
    JSON *prev = NULL;
    JSON *current = *head;
    JSON *next;

    while (current != NULL)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    *head = prev;
}

int list_length(JSON *head)
{
    int len = 0;

    for (JSON *p = head; p != NULL; p = p->next)
        len++;

    return len;
}

char *tokenizer(char *buf_in, char *token)
{
    char *buf       = rm_leading_whitespaces(buf_in);
    int   j         = 0;
    int   is_string = 0;
    int   size      = strlen(buf);
    int   i;

    for (i = 0; i < size; i++)
    {
        if (!is_string)
        {
            if (is_structural(buf[i])) // true
            {
                token[0] = buf[i];
                break;
            }
            else
            {
                if (buf[i] == '"')
                    is_string = 1;

                if (buf[i] != ' ')
                {
                    token[j] = buf[i];
                    j++;
                }
                if (is_structural(buf[i + 1]))
                {
                    j--;
                    break;
                }
            }
        }
        else
        {
            token[j] = buf[i];
            j++;

            if (buf[i] == '"')
            {
                is_string = 0;
                j--;
                break;
            }
        }
    }

    if (strlen(buf) == 0)
        return NULL;

    token[j + 1] = '\0';
    return buf + i + 1;
}

int is_whitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        return 1;
    return 0;
}

char *rm_leading_whitespaces(char *buf)
{
    char *p;
    for (p = buf; *p; p++)
    {
        if (!is_whitespace(*p))
            break;
    }
    return p;
}

int is_structural(char c)
{
    if (c == '[' || c == ',' || c == '{' || c == ']' || c == '}' || c == ':')
        return 1;
    return 0;
}

JSON *which_type(char *token)
{
    JSON *p;
    int   is_int = 1;
    int   len    = strlen(token);

    if (token[0] == '"')        // String
        p = parse_string(token);
    else if (strcmp(token, "true") == 0 || strcmp(token, "false") == 0)
        p = parse_boolean(token);
    else if (isdigit(token[0]))
    {
        for (int i = 0; i < len; i++)
        {
            if (token[i] == '.' || token[i] == 'e' || token[i] == 'E')
                is_int = 0;
        }

        if (is_int)
            p = parse_num_int(token);
        else
            p = parse_num_dbl(token);
    }
    else
    {
        printf("Unknown tokens\n");
        p = NULL;
    }

    return p;
}
