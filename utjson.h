#ifndef __UTJSON_H
#define __UTJSON_H

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

// Parse JSON
JSON *parse_boolean(char *buf);
JSON *parse_num_int(char *buf);
JSON *parse_num_dbl(char *buf);
JSON *parse_string(char *buf);
JSON *parse_array(char **buf);
JSON *parse_object(char **buf);
JSON *parse_json(char *buf);                // Parse all types of JSON

// Support parsing array
int   prepend_json(JSON **head, JSON *item);
void  reverse_json_list(JSON **head);
int   array_length(JSON *head);

// Support parsing object
KV   *object_kv(char **buf, char *key);     // Parse key value in object
int   prepend_kv(KV **head, KV *item);
void  reverse_kv_list(KV **head);
int   object_length(KV *head);

// Support parsing JSON in general
int   is_structural(char c);                // Check if token is structural
int   is_whitespace(char c);
char *rm_leading_whitespaces(char *buf);    // Delete whitespace in between
char *tokenizer(char *buf_in, char *token); // Take apart each tokens
char *show_type(JSON *item);                // Return string of each token type
int   token_type(char *token);              // Return enum to identify token type
void  print_json(JSON *json);
void  free_json(JSON *json);                // Free all mallocs

#endif
