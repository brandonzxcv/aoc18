#include "../include/br.h"
#include <string.h>

#define MAX_DATA 50000
static char data[MAX_DATA];
#define MAX_CHILDS 20

//part 1 sum
char * parse_childnode(char *cp, int *sum){
    int childNodeCount = atoi(cp);
    cp = strchr(cp, ' ') + 1;
    int metadataCount = atoi(cp);
    cp = strchr(cp, ' ') + 1;    

    for(int i = 0; i < childNodeCount; ++i){
        cp = parse_childnode(cp, sum);
    }

    //sum the metadata entries
    for(int i = 0; i < metadataCount; ++i){
        *sum += atoi(cp);
        cp = strchr(cp, ' ') + 1;
    }
    return cp;
}

//part 2 sum
char * parse_childnode_part2(char *cp, int *sum){
    int childNodeCount = atoi(cp);
    cp = strchr(cp, ' ') + 1;
    int metadataCount = atoi(cp);
    cp = strchr(cp, ' ') + 1;    

    int sums[MAX_CHILDS] = {0};
    //parse child nodes and store there sums
    for(int i = 0; i < childNodeCount; ++i){
        cp = parse_childnode_part2(cp, sums + i);
    }

    for(int i = 0; i < metadataCount; ++i){
        int mIndex = atoi(cp);
        //sum the child index if we have children
        if(childNodeCount){
            *sum += sums[mIndex - 1];
        } else { 
            //sum the metadata entries if no children
            *sum += mIndex;
        }
        cp = strchr(cp, ' ') + 1;
    }

    return cp;
}

int main(void){
    FILE *input = file_open("input.txt", "r");
    fread(data, 1, MAX_DATA, input);
    fclose(input);

    char *cp = data;
    //part 1
    int sum = 0;
    parse_childnode(cp, &sum);
    printf("Sum 1 = %i\n", sum);

    //part 2
    cp = data;
    sum = 0;
    parse_childnode_part2(cp, &sum);
    printf("Sum 2 = %i\n", sum);
}
