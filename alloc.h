#include <stdlib.h>

typedef struct{
    void *base;
    size_t offset;
    size_t size;
} LinearAlloc;

void LaInit(LinearAlloc *la, size_t size){
    la->size = size;
    la->base = malloc(size);
    la->offset = 0;
}

void * LaAlloc(LinearAlloc *la, size_t size){
    if(la->offset + size > la->size) {exit(1);}
    void *region = ((char *)la->base) + la->offset;
    la->offset += size;
    return region;
}