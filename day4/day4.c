#include <stdint.h>
#include <string.h>
#include "../include/br.h"

#define BIT_ASLEEP 0x8000
#define BIT_AWAKE 0x4000

typedef struct{
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint16_t data;
} TimeStamp;

typedef struct {
    uint8_t minutes[60];
    uint8_t lastMinute;
    uint16_t minutesAsleep;
} GuardInfo;

static TimeStamp timestamps[2048];
static int timestampCount;

#define MAX_GUARDS 4096
static GuardInfo guardInfo[MAX_GUARDS];

TimeStamp parse_timestamp(char *line){
    TimeStamp r = {0};
    line = strchr(line, '-') + 1;
    r.month = atoi(line);
    line = strchr(line, '-') + 1;
    r.day = atoi(line);
    line = strchr(line, ' ') + 1;
    r.hour = atoi(line);
    line = strchr(line, ':') + 1;
    r.minute = atoi(line);
    line = strchr(line, ' ') + 1;
    if(line[0] == 'f')
        r.data = BIT_ASLEEP;
    else if(line[0] == 'w')
        r.data = BIT_AWAKE;
    else {
        line = strchr(line, '#') + 1;
        r.data = atoi(line);
    }
    return r;
}

void print_timestamp(TimeStamp *t){
    printf("T:[%u-%u %u:%u]", t->month, t->day, t->hour, t->minute);
    if(t->data & BIT_ASLEEP)
        printf("ASLEEP\n");
    else if(t->data & BIT_AWAKE)
        printf("AWAKE\n");
    else
        printf("Guard #%u\n", t->data);
}

int cmp_timestamps_fnc(const void *a, const void *b){
    TimeStamp *t1 = ((TimeStamp*)a);
    TimeStamp *t2 = ((TimeStamp*)b);
    if(t1->month != t2->month) return t1->month - t2->month;
    if(t1->day != t2->day) return t1->day - t2->day;
    if(t1->hour != t2->hour) return t1->hour - t2->hour;
    return t1->minute - t2->minute;
}

int main(void){
    FILE *input = open_file("input.txt", "r");    

    char line[128];
    while(fgets(line, 128, input)){
        timestamps[timestampCount++] = parse_timestamp(line);
    }
    fclose(input);

    qsort(timestamps, timestampCount, sizeof(TimeStamp), cmp_timestamps_fnc);

    //keep track of guard with most minutes asleep
    int maxMinutesSlept = 0;
    int maxMinutesGuardId = 0;
    //keep track of max minutes guard most frequent minute
    int mostFreqMinute = 0;
    int mostFreqMinuteCount = 0;

    //keep track of the most frequent minute between all the guards
    int anyMostFreqMinute = 0;
    int anyMostFreqMinuteCount = 0;
    int anyMostFreqGuardId = 0;
    GuardInfo *info = NULL;
    for(int i = 0; i < timestampCount; ++i){
        TimeStamp *t = timestamps + i;
        if(i == 0) {
            info = guardInfo + t->data;
            continue;
        }
        if(t->data & BIT_ASLEEP){
            info->lastMinute = t->minute;
        } else if(t->data & BIT_AWAKE){
            info->minutesAsleep += t->minute - info->lastMinute;
            //check max minutes
            if(maxMinutesSlept < info->minutesAsleep){
                maxMinutesSlept = info->minutesAsleep;
                maxMinutesGuardId = info - guardInfo;
            }
            for(int m = info->lastMinute; m < t->minute; ++m){
                ++info->minutes[m];
                //if currently max minutes guard, check minute frequency
                if(maxMinutesGuardId == info - guardInfo){
                    if(info->minutes[m] > mostFreqMinuteCount){
                        mostFreqMinuteCount = info->minutes[m];
                        mostFreqMinute = m;
                    }
                }
                //check minute frequency amongst rest of guards
                if(info->minutes[m] > anyMostFreqMinuteCount){
                    anyMostFreqMinute = m;
                    anyMostFreqMinuteCount = info->minutes[m];
                    anyMostFreqGuardId = info - guardInfo;
                }   
            }
        } else{
            info = guardInfo + t->data;
        }
    }

    printf("Guard %i has slept for the most minutes : %i minutes.\n", maxMinutesGuardId, maxMinutesSlept);
    printf("They slept the most during minute %i. (%i minutes slept)\n", mostFreqMinute, mostFreqMinuteCount);
    printf("Part 1 answer = %i\n", maxMinutesGuardId * mostFreqMinute);
    //part 2
    printf("Guard %i has the most frequent minute: %i. (%i)\n", anyMostFreqGuardId, anyMostFreqMinute, anyMostFreqMinuteCount);
    printf("Part 2 = %i\n", anyMostFreqGuardId * anyMostFreqMinute);


}