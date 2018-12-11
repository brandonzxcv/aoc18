#include "../include/br.h"
#include <stdint.h>

static char output[26];
static int outputIndex;

#define DONE    0x80000000
#define WORKING 0x40000000
#define TIME    0x0FFFFFFF
static uint32_t dependencies[26];
static uint32_t lettersUsed;

#define P2
#ifdef P2
static int workersAvailable = 5;
static int addedTime = 60;

void part2(void){
    int time = 0;
    int dependenciesLeft = 1;
    while(dependenciesLeft){
        dependenciesLeft = 0;
        //check for any finished workers, and remove from other steps dependencies
        for(int i = 0; i < 26; ++i){        
            if(dependencies[i] == DONE || !(lettersUsed & (1 << i)))
                continue;

            if(dependencies[i] & WORKING){
                if(time >= (dependencies[i] & TIME)){
                    printf("Finished step %c at: %is\n", i + 'A', dependencies[i] & TIME);
                    output[outputIndex++] = i + 'A';
                    dependencies[i] = DONE;
                    //remove this dependency from other steps now that we have output it
                    for(int j = 0; j < 26; ++j){
                        if(!(dependencies[j] & WORKING))
                            dependencies[j] &= ~(1 << i);
                    }
                    ++workersAvailable;
                } else {
                    dependenciesLeft = 1;    
                }
            } else {
                dependenciesLeft = 1;
            }
        }        

        //check for steps that have no dependencies remaining and assign to available worker
        for(int i = 0; i < 26; ++i){   
            if(dependencies[i] == DONE || !(lettersUsed & (1 << i)))
                continue;
            //add work
            if(dependencies[i] == 0 && workersAvailable > 0){            
                dependencies[i] = time + i + addedTime + 1;
                dependencies[i] |= WORKING;
                --workersAvailable;
                dependenciesLeft = 1;
                printf("Assigned step %c to worker. Will be finished at: %i\n", i + 'A', dependencies[i] & TIME);
            }
        }
        ++time;
    }
   
    printf("Steps took %i seconds to complete.\n", time - 1);
}
#endif

void print_output(void){
    for(int i = 0; i < outputIndex; ++i){
        printf("%c", output[i]);
    }
    printf("\n");
}

int main(void){
    FILE *input = file_open("input.txt", "r");

    char line[64];
    while(fgets(line, 64, input)){
        if(line[0] != 'S')
            continue;
        //get letter index 'A' - 'Z' = 0 - 25
        char before = line[5] - 'A';
        char after = line[36] - 'A';

        dependencies[after] |= (1 << before);
        lettersUsed |= (1 << before) | (1 << after);
    }
    fclose(input);

    #ifdef P2
    part2();    
    return;
    #endif

    for(int i = 0; i < 26; ++i){        
        if(dependencies[i] == DONE || !(lettersUsed & (1 << i)))
            continue;

        //letter has no dependencies remaining, we can output it
        if(dependencies[i] == 0){
            output[outputIndex++] = i + 'A';
            dependencies[i] = DONE;
            //remove this dependency from other letters now that we have output it
            for(int j = 0; j < 26; ++j){
                dependencies[j] &= ~(1 << i);
            }
            //start again from the beginning
            i = -1;
        } 
    }

    print_output();
}