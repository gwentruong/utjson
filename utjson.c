#include <stdio.h>

int main(int argc, char **argv)
{
    char *file = argv[1];
    FILE *fp   = fopen(file, "r");
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

    for (int i = 0; i < size; i++)
        printf("%c", buf[i]);

    fclose(fp);
    return 0;
}
