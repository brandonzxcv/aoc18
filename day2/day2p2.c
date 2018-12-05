#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/alloc.h"

#define TABLE_SIZE 1024 //This should increase as the input size grows. (ex.100k inputs => 4096)
#define ALLOC_MEM (1024 * 1024 * 1)
#define ID_MAX_LEN 27 //Instead of 32, we can set this lower to skip the unused bits.
#define NULL_L 31

//Represents a string of 32 characters each with 32 possible values, as a sequence of 160 bits
//A single letter is encoded using 5 bits. 1 bit is stored in each uint32. 
//The uints are shifted to the left as more characters are encoded. 
//The strings first letter will be stored at the most significant bit and the last letter will at the least significant bit.
// ex.
//   10001010010000101010111110111111 = p1
//   01010011111001101001101001111111 = p2
//   01110010011010101111010010111111 = p3
//   10000100111110100001000110111111 = p4
//   01100000000100001000110101111111 = p5
// = jwugbihckpoymcpaxefotvdzns   ^
//   ^                            └ nulls
//   └  = 01001 = 9  ->  9 + 'a' = j
typedef struct {
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint32_t p4;
    uint32_t p5;
} Uint160;

struct IdNode{
    Uint160 id;
    struct IdNode *next;
};
typedef struct IdNode IdNode;

IdNode *idNodes[TABLE_SIZE] = {0};
LinearAlloc la = {0};

Uint160 StrToUint160(char *id){
    Uint160 r = {0};
    int c;
    for(int i = 0; i < ID_MAX_LEN; ++i){
        if(!id[i] || id[i] == '\n'){
            c = NULL_L;
        } else {
            c = id[i] - 'a';
        }

        r.p1 = (r.p1 << 1) | (c & 1);
        r.p2 = (r.p2 << 1) | ((c >> 1) & 1);
        r.p3 = (r.p3 << 1) | ((c >> 2) & 1);
        r.p4 = (r.p4 << 1) | ((c >> 3) & 1);
        r.p5 = (r.p5 << 1) | ((c >> 4) & 1);
    }

    return r;
}

//Using XOR we can find out if any letters are different.
//This will return 0 if both strings are equal
//We can count how many bits are set, to give us the number of letters that differ.
uint32_t Uint160Cmp(Uint160 a, Uint160 b){
    return (a.p1 ^ b.p1)
         | (a.p2 ^ b.p2)
         | (a.p3 ^ b.p3)
         | (a.p4 ^ b.p4)
         | (a.p5 ^ b.p5);
}

void PrintUint160Str(Uint160 a){
    char str[ID_MAX_LEN] = {0};
    int c;
    for(int i = 0; i < ID_MAX_LEN; ++i){
        c = ((a.p1 >> i) & 1)
            | (((a.p2 >> i) & 1) << 1)
            | (((a.p3 >> i) & 1) << 2)
            | (((a.p4 >> i) & 1) << 3)
            | (((a.p5 >> i) & 1) << 4);
        if(c == NULL_L){
            str[(ID_MAX_LEN-1) - i] = 0;
        } else {
            str[(ID_MAX_LEN-1) - i] = c + 'a';    
        }
    }
    printf("Text:%s\n", str);
}


void AddIdNode(Uint160 a){
    int i = a.p3 % TABLE_SIZE;
    IdNode **node = idNodes + i;
    checkNode:
    if(!*node){
        *node = (IdNode*)LaAlloc(&la, sizeof(IdNode));
        (*node)->id = a;
        (*node)->next = NULL;
        return;
    } else if(__popcnt(Uint160Cmp((*node)->id, a)) == 1){
        printf("IDs Found:\n");
        PrintUint160Str((*node)->id);
        PrintUint160Str(a);        
        exit(0);
    } else {
        node = &(*node)->next;
        goto checkNode;
    }
}

int main(int argc, char **argv){
    FILE *input = fopen("day2.input", "r");
    LaInit(&la, ALLOC_MEM);

    char line[ID_MAX_LEN] = {0};
    while(fgets(line, ID_MAX_LEN, input) != NULL){        
        AddIdNode(StrToUint160(line));
    }
    fclose(input);
}