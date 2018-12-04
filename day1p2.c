#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct{
    void *base;
    size_t offset;
    size_t size;
} LinearAlloc;

LinearAlloc la = {0};

void * LaAlloc(size_t size){
    if(la.offset + size > la.size) {exit(1);}
    void *region = ((char *)la.base) + la.offset;
    la.offset += size;
    return region;
}

#define MAX_MEM 1024 * 1024 * 10
#define FREQ_HASH_RANGE 1024

struct FreqBucket {
    int freq;
    struct FreqBucket *next;
};
typedef struct FreqBucket FreqBucket;

FreqBucket *freqHashTable[FREQ_HASH_RANGE] = {NULL};

void checkFreq(int f){
    FreqBucket **fb = freqHashTable + (f % FREQ_HASH_RANGE);
    checkBucket:
    if(!*fb){
        *fb = (FreqBucket*)LaAlloc(sizeof(FreqBucket));
        (*fb)->freq = f;
        (*fb)->next = NULL;
        return;
    } else if((*fb)->freq == f){
        printf("Frequency Reached Twice:%i", f);
        exit(0);
    } else {
        fb = &((*fb)->next);
        goto checkBucket;
    } 
}

int main(int argc, char **argv){
    FILE *input = fopen("day1.input", "r");  
    la.size = MAX_MEM;
    la.base = malloc(la.size);
    int c, f, d, n;
	c = f = d = n = 0;
    while(1){
        while((c = getc(input)) != EOF){
            #define INCR f+=d;if(d)checkFreq(f);d=0;
            switch(c){
                case '\n': INCR break;
                case '+': n = 0; break;
                case '-': n = 1; break;
                default: 
                    d = d * (d ? 10 : 1) + (n ? ('0' - c) : (c - '0'));
            }
        }
        INCR    
        fseek(input, 0, SEEK_SET);
    }
    free(la.base);
    fclose(input);
}