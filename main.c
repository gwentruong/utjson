#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "token.h"

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
