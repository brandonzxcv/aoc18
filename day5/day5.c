#include "../include/br.h"
#include <stdint.h>
#include <time.h>

#define MAX_POLYMER_LEN 60000
static char polymer[MAX_POLYMER_LEN];
static uint32_t polymerLen;

//flag characters removed during first phase
#define REMOVED_P1 1
static char polymerDelta[MAX_POLYMER_LEN];

//must be same letter but different case ex. (A and a) (c and C)
int is_reaction(char c1, char c2){
    return (c1 ^ c2) == 32;
}

int is_same_letter(char c1, char c2){
    return (c1 ^ c2) == 32 || (c1 ^ c2) == 0;
}

int sim_reaction(void){
    //keep track of last character that has not been removed. When a reaction occurs, move last character back until we find a unit, or if there are no units remaining going backwards, move it forwards to next unit
    uint32_t unitsDestroyed = 0;
    uint32_t lastCharIndex = 0;
    for(uint32_t i = 1; i < polymerLen; ++i){
        if(is_reaction(polymer[lastCharIndex], polymer[i])){
            polymerDelta[i] = REMOVED_P1;
            polymerDelta[lastCharIndex] = REMOVED_P1;
            unitsDestroyed += 2;
            while(lastCharIndex > 0 && polymerDelta[--lastCharIndex] == REMOVED_P1){}
            //no polymer unit found going backwards
            if(lastCharIndex == 0 && polymerDelta[lastCharIndex] == REMOVED_P1){
                lastCharIndex = ++i;
            }
        } else {
            lastCharIndex = i;
        }
    }

    return unitsDestroyed;
}

int sim_reaction2(char ignore){
    uint32_t unitsDestroyed = 0;
    uint32_t lastCharIndex = 0;

    for(uint32_t i = 0; i < polymerLen; ++i){
        //move lastCharIndex to first non removed/ignored character we find
        if(polymerDelta[lastCharIndex] == REMOVED_P1){
            ++lastCharIndex;
            continue;
        }
        if(is_same_letter(polymer[lastCharIndex], ignore)){
            ++lastCharIndex;
            ++unitsDestroyed;
            continue;
        }
        //ignore chars from removed from first phase
        if(polymerDelta[i] == REMOVED_P1){
            continue;
        }
        //ignore the current ignored character
        if(is_same_letter(polymer[i], ignore)){
            polymerDelta[i] = ignore;
            ++unitsDestroyed;
            continue;
        }

        if(is_reaction(polymer[lastCharIndex], polymer[i])){
            polymerDelta[i] = ignore;
            polymerDelta[lastCharIndex] = ignore;
            unitsDestroyed += 2;
            while(lastCharIndex > 0 && (polymerDelta[lastCharIndex] == REMOVED_P1 || polymerDelta[lastCharIndex] == ignore || is_same_letter(polymer[lastCharIndex], ignore))){
                --lastCharIndex;
            }
            //no polymer unit found going backwards
            if(lastCharIndex == 0 && (polymerDelta[lastCharIndex] == REMOVED_P1 || polymerDelta[lastCharIndex] == ignore || is_same_letter(polymer[lastCharIndex], ignore))){
                while(polymerDelta[lastCharIndex] == REMOVED_P1 || polymerDelta[lastCharIndex] == ignore || is_same_letter(polymer[lastCharIndex], ignore)) {
                    ++lastCharIndex;
                }

                lastCharIndex = ++i;
            }
        } else {
            lastCharIndex = i;
        }
    }

    return unitsDestroyed;
}

int main(void){
    FILE *input = open_file("input.txt", "r");
    polymerLen = fread(polymer, 1, MAX_POLYMER_LEN, input);
    fclose(input);
    //ignore new line at end
    if(polymer[polymerLen - 1] == '\n')
        --polymerLen;

    int destroyed = sim_reaction();
    int unitsLeft = polymerLen - destroyed;
    printf("Units destroyed = %u, Units Left = (%u - %u) = %u\n", destroyed, polymerLen, destroyed, unitsLeft);

    int maxDestroyed = 0;
    int maxDestroyedIgnore = 0;
    for(char i = 'a'; i <= 'z'; ++i){
        destroyed = sim_reaction2(i);
        if(destroyed > maxDestroyed){
            maxDestroyed = destroyed;
            maxDestroyedIgnore = i;
        }
        printf("Ignore ['%c'] => Units destroyed = %u\n", i, destroyed);
    }
    
    int unitsLeft2 = unitsLeft - maxDestroyed;
    printf("Part 2 Answer:\n");
    printf("Ignore ['%c'] =>Units destroyed = %u, Units left = (%u - %u) = %u\n", maxDestroyedIgnore, maxDestroyed, unitsLeft, maxDestroyed, unitsLeft2);
}