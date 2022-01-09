#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/helper.h"


int main() {
    unsigned long size;
    char *filename = "public/pig.txt";
    char *buffer = load_file(filename);
    size = strlen(buffer);
    printf("%lu\n", size);
    printf("%s\n", buffer);
    free(buffer);

    return 0;
}