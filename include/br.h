#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) (a > b) ? a : b;
#define MIN(a,b) (a < b) ? a : b;

typedef enum{
    EaseTypeInOutQuad,
    EaseTypeInQuad
} EaseType;

float T_EaseInQuad(float t){
    return t*t;
}
float T_EaseInOutQuad(float t) {
    return (t < 0.5f) ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

float Lerp(float start, float end, float t){
    return start + (end - start) * t;
}

FILE * file_open(char *path, char *mode){
    FILE *f = fopen(path, mode);
    if(!f){
        printf("Failed to open %s", path);
        exit(1);
    }
    return f;
}


typedef struct{
    void *base;
    size_t offset;
    size_t size;
} LinearAlloc;

void lalloc_init(LinearAlloc *la, size_t size){
    la->size = size;
    la->base = malloc(size);
    memset(la->base, 0, size);
    la->offset = 0;
}

void * lalloc(LinearAlloc *la, size_t size){
    if(la->offset + size > la->size) {exit(1);}
    void *region = ((char *)la->base) + la->offset;
    la->offset += size;
    return region;
}