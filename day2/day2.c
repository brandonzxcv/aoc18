#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv){
    FILE *input = fopen("input.txt", "r");
    int c;
    //Bits 25 - 0 represent a letter index, and are set if the letter has been hit
    //We have 3 uint32s to keep track of each count (1,2,3). if a bit in l[2] is set, the letter was hit 3 times. (l[1] and l[0] bits will also be set in this case)
    uint32_t letters[3] = {0};
    //Keeps track of letters that have counts of 2 and counts of 3 in the current ID by representing there letter index as a bit
    //A uint for each individual count (2 & 3)
    uint32_t currentHits[2] = {0};
    uint16_t totals[2] = {0};

    while((c = getc(input)) != EOF){
        switch(c){            
            case '\n': 
                totals[0] += currentHits[0] > 0; 
                totals[1] += currentHits[1] > 0;
                letters[0]=letters[1]=letters[2]=currentHits[0]=currentHits[1]=0;
                break;
            default:
                c = 1 << (c - 'a');
                for(int i = 0; i < 3; ++i) {
                    if(!(letters[i] & c)) {
                        if(i > 0) currentHits[i - 1] |= c;
                        letters[i] |= c;
                        break;
                    } else if(i > 0) {
                        currentHits[i - 1] &= ~c;
                    }
                }
        }
    }
    printf("Count of 2 = [%i], Count of 3 = [%i], Checksum = %i", totals[0], totals[1], totals[0] * totals[1]);
    fclose(input);
}
