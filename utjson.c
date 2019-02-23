#include <stdio.h>

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

    unsigned char buf[size];
    int nitems = fread(buf, sizeof(buf), 1, fp);
    if (nitems != 1)
    {
        printf("Failed to read\n");
        return -1;
    }

    for (i = 0; i < size; i++)
        printf("%c", buf[i]);

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
