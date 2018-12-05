#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_DATA (1024 * 1024 * 3)
#define HASH_RANGE (1 << 16) //16 bit hash, based off first 4 chars
#define HASH_BUCKETS 32 //this may need to increase if we encounter a lot of common id hashes
#define LINE_LEN 26 //we can calculate this ourselves, but whatever

static char data[MAX_DATA];
static char *hashmap[HASH_RANGE][HASH_BUCKETS];

//Strings must be at least 8 characters long, since the hashes are based off the first 4 chars and 4 chars after its middle
uint16_t Hash4Chars(char *str) {
    return (str[0] << 12) 
         ^ (str[1] << 8) 
         ^ (str[2] << 4) 
         ^  str[3];
}

int PopcntStr(char *a, char *b) {
    int pc = 0;
    //early exit if we hit a popcount of 2, since we only care about 1 char diffs
    for (int i = 0; i < LINE_LEN && pc < 2; i++)
        if (a[i] != b[i]) ++pc;
    return pc;
}

int main(void){
    FILE *input = fopen("day2_100k.input", "r");
    //read all the ids in at once, to avoid any IO overhead
    size_t bytesRead = fread(data, 1, MAX_DATA, input);
    if(bytesRead <= 0){
        printf("Error reading file.\n");
      return;
    } 
    fclose(input);
    
    int numLines = bytesRead / LINE_LEN;
    //Generate two hashes. One hash for the ids first 4 chars, second hash for the 4 chars starting from the middle
    //We will insert both hashes into the hashmap for each id
    //The single char difference is going to be on one side of the id string, so we will get a hash collision on the other common half
    //We can then do a string compare on only these hash collisions to determine if there are any ids with a difference of 1 char
    //NOTE: This will not work for detecting more than 1 char diffs, since both sides could contain a diff
    uint16_t hash[2];
    char *line;
    char *id;
    for (int i = 0; i < numLines; i++) {
        line = &data[i * (LINE_LEN+1)];
        hash[0] = Hash4Chars(line);
        hash[1] = Hash4Chars(line + (LINE_LEN/2));

        for (int hi = 0; hi < 2; ++hi) {
            for (int bi = 0; bi < HASH_BUCKETS; ++bi) {
                id = hashmap[hash[hi]][bi];
                if (!id) {
                    hashmap[hash[hi]][bi] = line;
                    break;
                } else if (PopcntStr(line, id) == 1) {
                    printf("IDs Found:\n");
                    line[LINE_LEN] = 0;
                    printf("%s\n", line);
                    id[LINE_LEN] = 0;
                    printf("%s\n", id);
                    return 0;
                }
            }
        }
    }
    printf("Not Found\n");
}

