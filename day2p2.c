#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"

#define ID_LEN (27)

struct QuadWord{
    //q0
    uint32_t l0:5;
    uint32_t l1:5;
    uint32_t l2:5;
    uint32_t l3:5;
    uint32_t l4:5;
    uint32_t l5:5;
    uint32_t l6:5;
    uint32_t l7:5;
    uint32_t l8:5;
    uint32_t l9:5;
    uint32_t l10:5;
    uint32_t l11:5;
    //q1
    uint32_t l12:5;
    uint32_t l13:5;
    uint32_t l14:5;
    uint32_t l15:5;
    uint32_t l16:5;
    uint32_t l17:5;
    uint32_t l18:5;
    uint32_t l19:5;
    uint32_t l20:5;
    uint32_t l21:5;
    uint32_t l22:5;
    uint32_t l23:5;
    //q2
    uint32_t l24:5;
    uint32_t l25:5;
    uint32_t l26:5;
    //....unused
};
typedef struct QuadWord QuadWord;

struct IdBucket {
    uint32_t h;
    char id[ID_LEN];
    struct IdBucket *next;
};
typedef struct IdBucket IdBucket;

IdBucket *colBuckets[ID_LEN] = {0};
LinearAlloc la = {0};

uint32_t IdHash(char *id){
    uint32_t h = 0;
    for(int i = 0; i < ID_LEN; ++i){
        h |= 1 << (id[i] - 'a');
    }
    return h;
}

QuadWord idToQuadWord(char *id){
    char *c = id;
    l0 = c - 'a';
}

void AddIdHash(int col, char *id){
    //printf("%s\n", id);
    IdBucket **bucket = colBuckets + col;
    uint32_t idh = IdHash(id);
    checkBucket:    
    if(!*bucket){
        *bucket = (IdBucket*)LaAlloc(&la, sizeof(IdBucket));
        strncpy((*bucket)->id, id, ID_LEN + 1);        
        (*bucket)->h = idh;
        (*bucket)->next = NULL;
        return;
    } else if((*bucket)->h == idh && !strcmp((*bucket)->id, id)) {
        printf("IDs Found:\n");
        printf("%s\n", (*bucket)->id);
        printf("%s\n", id);
        exit(0);
    } else {
        bucket = &((*bucket)->next);
        goto checkBucket;
    }
}

int main(int argc, char **argv){
    printf("size:%i", sizeof(struct QuadWord));
    return;
    FILE *input = fopen("day2_10000.txt", "r");
    LaInit(&la, 1024 * 1024 * 10);

    char line[ID_LEN + 1];
    while(fgets(line, ID_LEN + 1, input) != NULL){
        for(int i = 0; i < ID_LEN - 1; ++i){
            char o = line[i];
            line[i] = '{';
            line[ID_LEN - 1] = 0;
            AddIdHash(i, line);
            line[i] = o;
        }
    }
    fclose(input);
}