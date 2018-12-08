#include <stdio.h>
#include <stdlib.h>

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

FILE * open_file(char *path, char *mode){
    FILE *f = fopen(path, mode);
    if(!f){
        printf("Failed to open %s", path);
        exit(1);
    }
    return f;
}