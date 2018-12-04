#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "alloc.h"

#define MAX_MEM 1024 * 1024 * 10
#define FREQ_HASH_RANGE 1024

LinearAlloc la = {0};

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
        *fb = (FreqBucket*)LaAlloc(&la, sizeof(FreqBucket));
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
    LaInit(&la, MAX_MEM);
    int c, f, d, n;
	c = f = d = n = 0;
    while(1){
        while((c = getc(input)) != EOF){
            switch(c){
                case '\n': f+=d;if(d)checkFreq(f);d=0; break;
                case '+': n = 0; break;
                case '-': n = 1; break;
                default: 
                    d = d * (d ? 10 : 1) + (n ? ('0' - c) : (c - '0'));
            }
        }
        fseek(input, 0, SEEK_SET);
    }
    free(la.base);
    fclose(input);
}