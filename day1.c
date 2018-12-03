#include <stdio.h>

int main(int argc, char **argv){
    char c;
    int f = 0;
    int d = 0;
    int n = 0;
    while((c = getc(stdin)) != EOF){
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

    printf("Freq:%i\n", f);
}