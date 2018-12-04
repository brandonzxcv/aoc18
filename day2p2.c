#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv){
    FILE *input = fopen("day2.input", "r");

    uint32_t cletters[1024] = {0};
    char line[30];
    int ln = 0;
    while(fgets(line, 30, input) != NULL){
        char *c = line;
        while(*c & *c != '\n'){ cletters[ln] |= (1 << ((*c) - 'a')); ++c; }
        printf("%i:%s", ln, line);
        ++ln;
    }

    for(int i = 0; i < 1024 - 1; ++i){
        if(!cletters[i]) break;
        for(int j = i + 1; j < 1024; ++j){     
            if(!cletters[j]) break;  
            if(cletters[i] & cletters[j] == cletters[i] && cletters[j] & cletters[i] == cletters[j]){
                printf("%i with %i match\n", i + 1, j + 1);
            }
        }
    }

    //TODO(brandon): get BIT AND both ways between letters and check if its a power of 2. 
    //Which should give us an optimization hint that there is a 1 character difference, but no info about columns or repeating characters


    fclose(input);

    return;
    // int c;
    // uint32_t l[5] = {0};
    // uint16_t counts[2] = {0};

    // while((c = getc(input)) != EOF){
    //     switch(c){
    //         case '\n': counts[0] += l[3] > 0; counts[1] += l[4] > 0; l[0]=l[1]=l[2]=l[3]=l[4]=0; break;
    //         default:
    //             c = 1 << (c - 'a');
    //             for(int i = 0; i < 3; ++i) {
    //                 if(!(l[i] & c)) {
    //                     if(i == 1) l[3] |= c;
    //                     if(i == 2) l[4] |= c;
    //                     l[i] |= c;
    //                     break;
    //                 } else if(i > 0) {
    //                     if(i == 1) l[3] &= ~c;
    //                     if(i == 2) l[4] &= ~c;
    //                 }
    //             }
    //     }
    // }
    // printf("Count of 2 = [%i], Count of 3 = [%i], Checksum = %i", counts[0], counts[1], counts[0] * counts[1]);
    // fclose(input);
}