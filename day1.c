#include <stdio.h>

int main(int argc, char **argv){
    FILE *input = fopen("day1.input", "r");
    int c, f, d, n;
    c = f = d = n = 0;
    while((c = getc(input)) != EOF){
        #define INCR f+=d;d=0;
        switch(c){
            case '\n':
            case '+': n = 0; INCR break;
            case '-': n = 1; INCR break;
            default: 
                d = d * (d ? 10 : 1) + (n ? ('0' - c) : (c - '0'));
        }
    }
    INCR
    printf("Frequency:%i\n", f);
    fclose(input);
}