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
    JSON *val;
    struct kv *next;
} KV;

JSON *parse_json(char *buf);
JSON *parse_boolean(char *buf);
JSON *parse_num_int(char *buf);
JSON *parse_num_dbl(char *buf);
JSON *parse_string(char *buf);
JSON *parse_array(char **buf);
JSON *parse_object(char **buf);
int   prepend_json(JSON **head, JSON *item);
void  reverse_json_list(JSON **head);
int   array_length(JSON *head);
char *tokenizer(char *buf_in, char *token);
int   is_whitespace(char c);
char *rm_leading_whitespaces(char *buf);
int   is_structural(char c);
char *show_type(JSON *item);
int   token_type(char *token);
void print_json(JSON *json);
KV   *object_kv(char **buf, char *key);
int   prepend_kv(KV **head, KV *item);
void  reverse_kv_list(KV **head);
int   object_length(KV *head);
void  free_json(JSON *json);

int main(int argc, char **argv)
{
    char *file = argv[1];
    FILE *fp   = fopen(file, "r");
    int   i;
    if (fp == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    printf("Size of the file %d\n", size);
    fseek(fp, 0L, SEEK_SET);

    char buf[size + 1];
    int nitems = fread(buf, size, 1, fp);
    if (nitems != 1)
    {
        printf("Failed to read\n");
        return -1;
    }
    buf[size] = '\0';

    for (i = 0; i < size; i++)
        printf("%c", buf[i]);

    JSON *js = parse_json(buf);

#if 0
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
#endif

    free_json(js);
    fclose(fp);
    return 0;
}

JSON *parse_boolean(char *token)
{
    JSON *boolean = malloc(sizeof(JSON));

    boolean->type = JSON_BOOLEAN;
    boolean->size = 1;
    boolean->data = malloc(1);
    if (strcmp(token, "true") == 0)
        *(char *)boolean->data = 1;
    else if (strcmp(token, "false") == 0)
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
    string->data = strdup(str);
    string->next = NULL;

    return string;
}

JSON *parse_array(char **buf)
{
    char  token[8192] = { '\0' };
    char *p = *buf;
    JSON *head = NULL;
    JSON *e;
    int   t;

    JSON *array = malloc(sizeof(JSON));

    array->type = JSON_ARRAY;
    array->next = NULL;

    while ((p = tokenizer(p, token)) != NULL)
    {
        t = token_type(token);
        if (t == JSON_NUM_INT)
        {
            e = parse_num_int(token);
            prepend_json(&head, e);
        }
        else if (t == JSON_NUM_DBL)
        {
            e = parse_num_dbl(token);
            prepend_json(&head, e);
        }
        else if (t == JSON_BOOLEAN)
        {
            e = parse_boolean(token);
            prepend_json(&head, e);
        }
        else if (t == JSON_STRING)
        {
            e = parse_string(token);
            prepend_json(&head, e);
        }
        else if (token[0] == ',')
            ;
        else if (token[0] == '[')
        {
            e = parse_array(&p);
            prepend_json(&head, e);
        }
        else if (token[0] == '{')
        {
            e = parse_object(&p);
            prepend_json(&head, e);
        }
        else if (token[0] == ']')
        {
            reverse_json_list(&head);
            array->size = array_length(head);
            array->data = head;
            *buf = p;
            break;
        }
    }
    return array;
}

int prepend_json(JSON **head, JSON *item)
{
    if (item == NULL)
        return -1;

    item->next = *head;
    *head = item;

    return 0;
}

void reverse_json_list(JSON **head)
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

int array_length(JSON *head)
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

char *show_type(JSON *item)
{
    switch (item->type)
    {
        case JSON_OBJECT:
            return "JSON_OBJECT";
        case JSON_ARRAY:
            return "JSON_ARRAY";
        case JSON_BOOLEAN:
            return "JSON_BOOLEAN";
        case JSON_NUM_INT:
            return "JSON_NUM_INT";
        case JSON_NUM_DBL:
            return "JSON_NUM_DBL";
        case JSON_STRING:
            return "JSON_STRING";
        default:
            return "Unknown";
    }
}

int token_type(char *token)
{
    int is_int = 1;
    int len    = strlen(token);

    if (token[0] == '"')
        return JSON_STRING;
    else if (strcmp(token, "true") == 0 || strcmp(token, "false") == 0)
        return JSON_BOOLEAN;
    else if (isdigit(token[0]))
    {
        for (int i = 0; i < len; i++)
        {
            if (token[i] == '.' || token[i] == 'e' || token[i] == 'E')
                is_int = 0;
        }

        if (is_int)
            return JSON_NUM_INT;
        else
            return JSON_NUM_DBL;
    }
    else if (strcmp(token, "[") == 0)
        return JSON_ARRAY;
    else if (strcmp(token, "{") == 0)
        return JSON_OBJECT;

    return -1;
}

void print_json(JSON *json)
{
    switch(json->type)
    {
        case JSON_ARRAY:
            printf("%s\n", show_type(json));
            for (JSON *p = json->data; p != NULL; p = p->next)
                print_json(p);
            break;
        case JSON_BOOLEAN:
            printf("%s (%s)\n", show_type(json),
                   *(int *)json->data ? "true" : "false");
            break;
        case JSON_NUM_INT:
            printf("%s (%d)\n", show_type(json), *(int *)json->data);
            break;
        case JSON_NUM_DBL:
            printf("%s (%f)\n", show_type(json), *(double *)json->data);
            break;
        case JSON_STRING:
            printf("%s (%s)\n", show_type(json), json->data);
            break;
        case JSON_OBJECT:
            printf("%s\n", show_type(json));
            for (KV *p = json->data; p!= NULL; p = p->next)
            {
                printf("%s -> ", p->key);
                print_json(p->val);
            }
            break;
        default:
            printf("Unknown\n");
            break;
    }
}

JSON *parse_object(char **buf)
{
    char  token[8192] = { '\0' };
    char *p    = *buf - 1;
    KV   *head = NULL;
    KV   *e;
    JSON *object = malloc(sizeof(JSON));

    object->type = JSON_OBJECT;
    object->next = NULL;

    while((p = tokenizer(p, token)) != NULL)
    {
        if (token[0] == '"')
        {
            e = object_kv(&p, token);
            prepend_kv(&head, e);
        }
        else if (token[0] == ',')
            ;
        else if (token[0] == '}')
        {
            reverse_kv_list(&head);
            object->size = object_length(head);
            object->data = head;
            *buf = p;
            break;
        }
    }

    return object;
}

KV *object_kv(char **buf, char *key)
{
    char token[8192] = { '\0' };
    KV  *key_value   = malloc(sizeof(KV));
    char *p          = *buf;    
    JSON *value;
    int t;

    while ((p = tokenizer(p, token)) != NULL)
    {
        t = token_type(token);
        if (t == JSON_NUM_INT)
            value = parse_num_int(token);
        else if (t == JSON_NUM_DBL)
            value = parse_num_dbl(token);
        else if (t == JSON_BOOLEAN)
            value = parse_boolean(token);
        else if (t == JSON_STRING)
            value = parse_string(token);
        else if (t == JSON_ARRAY)
            value = parse_array(&p);
        else if (t == JSON_OBJECT)
            value = parse_object(&p);
        else if (token[0] == ':')
            ;
        else if (token[0] == ',' || token[0] == '}')
            break;
    }

    key_value->key = strdup(key);
    key_value->next = NULL;
    key_value->val = value;
    *buf = p - 1;

    return key_value;
}

int prepend_kv(KV **head, KV *item)
{
    if (item == NULL)
        return -1;

    item->next = *head;
    *head = item;

    return 0;
}

void reverse_kv_list(KV **head)
{
    KV *prev = NULL;
    KV *current = *head;
    KV *next;

    while (current != NULL)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    *head = prev;
}

int object_length(KV *head)
{
    int len = 0;

    for (KV *p = head; p != NULL; p = p->next)
        len++;

    return len;
}

JSON *parse_json(char *buf)
{
    char  token[8192] = { '\0' };
    char *p = buf;
    JSON *js;
    int   t;

    while ((p = tokenizer(p, token)) != NULL)
    {
        t = token_type(token);
        if (t == JSON_OBJECT)
            js = parse_object(&p);
        else if (t == JSON_ARRAY)
            js = parse_array(&p);
        else if (t == JSON_BOOLEAN)
            js = parse_boolean(token);
        else if (t == JSON_NUM_INT)
            js = parse_num_int(token);
        else if (t == JSON_NUM_DBL)
            js = parse_num_dbl(token);
        else if (t == JSON_STRING)
            js = parse_string(token);
    }
    print_json(js);

    return js;
}

void free_json(JSON *json)
{
    KV   *head_kv;
    JSON *head_js;

    switch (json->type)
    {
        case JSON_OBJECT:
            head_kv = json->data;
            for (KV *kv = head_kv; kv != NULL; kv = head_kv)
            {
                head_kv = head_kv->next;
                free(kv->key);
                free_json(kv->val);
                free(kv);
            }
            break;
        case JSON_ARRAY:
            head_js = json->data;
            for (JSON *js = head_js; js != NULL; js = head_js)
            {
                head_js = head_js->next;
                free_json(js);
            }
            break;
        default:
            free(json->data);
            break;
    }
    free(json);
}
