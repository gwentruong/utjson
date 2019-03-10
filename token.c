#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"

int is_structural(char c)
{
    if (c == '[' || c == ',' || c == '{' || c == ']' || c == '}' || c == ':')
        return 1;
    return 0;
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
