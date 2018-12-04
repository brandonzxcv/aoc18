#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv){
    FILE *input = fopen("day2.input", "r");
    int c;
    uint32_t l[5] = {0};
    uint16_t counts[2] = {0};

    while((c = getc(input)) != EOF){
        switch(c){
            case '\n': counts[0] += l[3] > 0; counts[1] += l[4] > 0; l[0]=l[1]=l[2]=l[3]=l[4]=0; break;
            default:
                c = 1 << (c - 'a');
                for(int i = 0; i < 3; ++i) {
                    if(!(l[i] & c)) {
                        if(i == 1) l[3] |= c;
                        if(i == 2) l[4] |= c;
                        l[i] |= c;
                        break;
                    } else if(i > 0) {
                        if(i == 1) l[3] &= ~c;
                        if(i == 2) l[4] &= ~c;
                    }
                }
        }
    }
    printf("Count of 2 = [%i], Count of 3 = [%i], Checksum = %i", counts[0], counts[1], counts[0] * counts[1]);
    fclose(input);
}