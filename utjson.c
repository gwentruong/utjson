#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} JSON;

typedef struct kv {
    char *key;
    void *val;
} KV;

JSON *parse_boolean(char *buf);
JSON *parse_num_int(char *buf);
JSON *parse_num_dbl(char *buf);
JSON *parse_string(char *buf);

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
    printf("Read file\n");

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

    // Parse num double
    // JSON *num_dbl = parse_num_dbl(buf);
    // printf("### Type %d Size %d Data %.3f\n", num_dbl->type, num_dbl->size, *(double *)num_dbl->data);

    // Parse string
    // char buf_str[8192];
    // for (i = 1, j = 0; i < size; i++, j++)
    // {
    //     if (buf[i] != '"')
    //         buf_str[j] = buf[i];
    //     else
    //         buf_str[j] = '\0';
    // }
    // JSON *str = parse_string(buf_str);
    // printf("### Type %d Size %d Data %s\n", str->type, str->size, str->data);

    // Parse boolean
    char buf_str[8192];
    for (i = 0; i < size; i++)
    {
        if (buf[i] != '\n')
            buf_str[i] = buf[i];
        else
            buf_str[i] = '\0';
    }
    JSON *boolean = parse_boolean(buf_str);
    printf("### Type %d Size %d Data %i\n", boolean->type, boolean->size, *(char *)boolean->data);
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

    free(boolean->data);
    free(boolean);
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

    return boolean;
}

JSON *parse_num_int(char *buf)
{
    JSON *num_int = malloc(sizeof(JSON));

    num_int->type = JSON_NUM_INT;
    num_int->size = sizeof(int);
    num_int->data = malloc(sizeof(int));
    *(int *)num_int->data = atoi(buf);

    return num_int;
}

JSON *parse_num_dbl(char *buf)
{
    JSON *num_dbl = malloc(sizeof(JSON));
    char *ptr;

    num_dbl->type = JSON_NUM_DBL;
    num_dbl->size = sizeof(double);
    num_dbl->data = malloc(sizeof(double));
    *(double *)num_dbl->data = strtod(buf, &ptr);

    return num_dbl;
}

JSON *parse_string(char *str)
{
    JSON *string  = malloc(sizeof(JSON));

    string->type = JSON_STRING;
    string->size = strlen(str);
    string->data = str;

    return string;
}
