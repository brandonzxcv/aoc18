#include "../include/br.h"
#include <stdint.h>
#include <inttypes.h>

#define BITS_PER_INDEX 32
#define BIT_ARRAY_SIZE (8)
#define MAX_BITS (BIT_ARRAY_SIZE * BITS_PER_INDEX)
static uint32_t state[BIT_ARRAY_SIZE];
static uint32_t state2[BIT_ARRAY_SIZE];
static uint32_t *currentstate;
static uint32_t *newstate;
static uint8_t bitmask[32];

#define NEG_POTS 6

void set_bit(uint32_t *bits, int i){
    int ai = i / BITS_PER_INDEX;
    int abi = i % BITS_PER_INDEX;
    bits[ai] |= (1 << abi);
}

void clear_bit(uint32_t *bits, int i){
    int ai = i / BITS_PER_INDEX;
    int abi = i % BITS_PER_INDEX;
    bits[ai] &= ~(1 << abi);
}

int test_bit(uint32_t *bits, int i){
    int ai = i / BITS_PER_INDEX;
    int abi = i % BITS_PER_INDEX;
    return (bits[ai] & (1 << abi)) != 0;
}

void parse_state(char *line){
    int index = NEG_POTS;
    while(*line){
        if(*line == '#')
            set_bit(state, index);
        ++line;
        ++index;
    }
}

void parse_mask(char *line){
    uint8_t key = 0;

    for(int i = 0; i < 5; ++i){
        if(*line == '#')
            key |= (1 << i);
        ++line;
    }

    if(strchr(line, '#')){
        bitmask[key] = 1;
        for(int i = 0; i < 5; ++i){
            if(key & (1 << i))
                printf("#");
            else
                printf(".");
        }
        printf("\n");
    }
}

int get_5bits(uint32_t *bits, int x){    
    int value = 0;
    value |= (test_bit(bits, x-2) << 0);
    value |= (test_bit(bits, x-1) << 1);
    value |= (test_bit(bits, x) << 2);
    value |= (test_bit(bits, x+1) << 3);
    value |= (test_bit(bits, x+2) << 4);
    return value;
}

void print_state(uint32_t *bits){
    for(int i = 0; i < MAX_BITS; ++i){
        if(test_bit(bits, i)){
            printf("#");
        } else {
            printf(".");
        }
    }
    printf("\n");
}

int sum_state(uint32_t *bits, int startingPot){
    int sum = 0;
    for(int i = 2; i < MAX_BITS; ++i){
        if(test_bit(currentstate, i)){
            sum += startingPot + i;
        }
    }
    return sum;
}

int main(void){
    FILE *input = file_open("input.txt", "r");
    char line[128];
    while(fgets(line, 128, input)){
        if(!strncmp(line, "initial state:", 14)){
            parse_state(line + 15);
        } else if(line[0] == '#' || line[0] == '.'){
            parse_mask(line);
        }
    }
    fclose(input);

    printf("0: ");
    print_state(state);

    currentstate = state;
    newstate = state2;
    
    int sameSumCount = 0;
    uint64_t lastSum = 0;
    int sumDiff = 0;
    int startingPot = -NEG_POTS;
    uint64_t generations = 50000000000;
    uint64_t gen = 1;
    for(gen = 1; gen <= generations; ++gen){
        int hitFirst = 0;
        int i2 = NEG_POTS;
        int sum = 0;
        for(int i = 2; i < MAX_BITS; ++i){ //start at index 2 since we need at least 2 pots on the left to do bitmask check
            if(bitmask[get_5bits(currentstate, i)]){
                if(!hitFirst){
                    hitFirst = 1;
                    startingPot = startingPot + (i - NEG_POTS);
                }
                sum += startingPot + i2;
                set_bit(newstate, i2);
                
            } else {
                clear_bit(newstate, i2);
            }
            if(hitFirst)
                ++i2;
        }
        // printf("%"PRIu64": ", gen);
        // print_state(newstate);
        if(sum - lastSum == sumDiff){
            ++sameSumCount;
        } else {
            sameSumCount = 0;
        }
        
        sumDiff = sum - lastSum;
        lastSum = sum;

        if(sameSumCount > 100){
            lastSum += (generations - gen) * sumDiff;
            printf("Sum = %"PRIu64, lastSum);
            return;
        }
        
        uint32_t *swap = currentstate;
        currentstate = newstate;
        newstate = swap;
    }

    printf("Sum = %"PRIu64, lastSum);
   
}