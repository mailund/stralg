#include <io.h>
#include <stdio.h>
#include <stdlib.h>

char *load_file(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); // rewinding

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    
    string[fsize] = 0;
    fclose(f);

    return string;
}
