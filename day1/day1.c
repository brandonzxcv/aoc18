#include <stdio.h>

int main(int argc, char **argv){
    FILE *input = fopen("day1.input", "r");
    int c, f, d, n;
    c = f = d = n = 0;
    while((c = getc(input)) != EOF){
        switch(c){
            case '\n': f+=d;d=0; break;
            case '+': n = 0; break;
            case '-': n = 1; break;
            default: 
                d = d * (d ? 10 : 1) + (n ? ('0' - c) : (c - '0'));
        }
    }
    printf("Frequency:%i\n", f);
    fclose(input);
}