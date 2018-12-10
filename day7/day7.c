#include "../include/br.h"
#include <stdint.h>

static char output[26];
static int outputIndex;

#define DONE 0x80000000
static uint32_t dependencies[26];
static uint32_t lettersUsed;

void print_output(void){
    for(int i = 0; i < outputIndex; ++i){
        printf("%c", output[i]);
    }
    printf("\n");
}

#define MAX_WORKERS 2
static uint32_t workers[MAX_WORKERS];

void part2(void){
    int time = 0;
    for(int i = 0; i < 26; ++i){        
        if(dependencies[i] == DONE || !(lettersUsed & (1 << i)))
            continue;

        //letter has no dependencies remaining, we can output it
        if(dependencies[i] == 0){            
            int foundWorker = 0;
            int leastWorkTime = INT32_MAX;
            for(int w = 0; w < MAX_WORKERS; ++w){
                if(workers[w] <= time){
                    //workers[w] = 60 + i + 1;
                    workers[w] = time + (i);
                    foundWorker = 1;
                    break;
                }
                if(workers[w] < leastWorkTime){
                    leastWorkTime = workers[w];
                }
            }            
            if(!foundWorker){
                time = leastWorkTime;
                i = -1;
                continue;
            }            
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
    printf("%i seconds.\n", time);
}

int main(void){
    FILE *input = file_open("test.txt", "r");

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

    part2();    
    return;

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
//AEMPOJNWIZCFSUKBDXQTHVLGRY
//APOJENWIZCFSMUKBDXQTHVLGRY
//AEMNPOJWZDFISUXCKQBTVHLGRY
//AEMNPOJWISZCDFUKBXQTHVLGRY
//AEMNPOJWISZCDFUKBXQTHVLGRY
//AEMNPOJWISZCDFUKBXQTHVLGRY
//AEMNPOJWISZCDFUKBXQTHVLGRY
/*--- Part Two ---
As you're about to begin construction, four of the Elves offer to help. "The sun will set soon; it'll go faster if we work together." Now, you need to account for multiple people working on steps simultaneously. If multiple steps are available, workers should still begin them in alphabetical order.

Each step takes 60 seconds plus an amount corresponding to its letter: A=1, B=2, C=3, and so on. So, step A takes 60+1=61 seconds, while step Z takes 60+26=86 seconds. No time is required between steps.

To simplify things for the example, however, suppose you only have help from one Elf (a total of two workers) and that each step takes 60 fewer seconds (so that step A takes 1 second and step Z takes 26 seconds). Then, using the same instructions as above, this is how each second would be spent:

Second   Worker 1   Worker 2   Done
   0        C          .        
   1        C          .        
   2        C          .        
   3        A          F       C
   4        B          F       CA
   5        B          F       CA
   6        D          F       CAB
   7        D          F       CAB
   8        D          F       CAB
   9        D          .       CABF
  10        E          .       CABFD
  11        E          .       CABFD
  12        E          .       CABFD
  13        E          .       CABFD
  14        E          .       CABFD
  15        .          .       CABFDE
Each row represents one second of time. The Second column identifies how many seconds have passed as of the beginning of that second. Each worker column shows the step that worker is currently doing (or . if they are idle). The Done column shows completed steps.

Note that the order of the steps has changed; this is because steps now take time to finish and multiple workers can begin multiple steps simultaneously.

In this example, it would take 15 seconds for two workers to complete these steps.

With 5 workers and the 60+ second step durations described above, how long will it take to complete all of the steps?
*/