#include <stdio.h>
#include <stdlib.h>

#define MAX(a,b) (a > b) ? a : b;
#define MIN(a,b) (a < b) ? a : b;

FILE * open_file(char *path, char *mode){
    FILE *f = fopen(path, mode);
    if(!f){
        printf("Failed to open %s", path);
        exit(1);
    }
    return f;
}