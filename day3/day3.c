#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BOXES 4096

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t x2;
    uint32_t y2;    
} Aabb;

Aabb ParseAabb(char *line){
    Aabb result = {0};
    line = strchr(line, '@') + 1;
    result.x = atoi(line);
    line = strchr(line, ',') + 1;
    result.y = atoi(line);
    line = strchr(line, ':') + 1;
    result.x2 = result.x + atoi(line);
    line = strchr(line, 'x') + 1;
    result.y2 = result.y + atoi(line);
    return result;
}

int AreIntersecting(Aabb *box1, Aabb *box2){
    return !(box2->x >= box1->x2 || box1->x >= box2->x2 || box2->y >= box1->y2 || box1->y >= box2->y2);
}

int GetIntersection(Aabb *a, Aabb *b, Aabb *out){
    if(!AreIntersecting(a, b)) return 0;

    out->x = (a->x < b->x) ? b->x : a->x;
    out->x2 = (a->x2 < b->x2) ? a->x2 : b->x2;
    out->y = (a->y < b->y) ? b->y : a->y;
    out->y2 = (a->y2 < b->y2) ? a->y2 : b->y2;

    return 1;
}

int Contains(Aabb *a, Aabb *b){
    return (b->x >= a->x && b->x2 <= a->x2 && b->y >= a->y && b->y2 <= a->y2);
}

int GetArea(Aabb *a){
    return (a->x2 - a->x) * (a->y2 - a->y);
}

void FlagAabb(Aabb *b){
    b->x = UINT32_MAX;
}

int IsFlag(Aabb *b){
    return b->x == UINT32_MAX;
}

static Aabb boxes[MAX_BOXES];
static int boxCount;

static Aabb intersections[MAX_BOXES];
static int intersectionCount;
static int areaSum;

// void TakeSpace(Aabb *b){
//     int areaLeft = AabbArea(b);
//     Aabb intersection = {0};
//     for(int i = 0; i < tboxCount; ++i){
//         Aabb *tb = tboxes + i;
//         if(AabbAreIntersecting(tb, b)){
//             intersection = AabbGetIntersection(tb, b);
//             //areaLeft
//         }
//     }
// }
int SortFunc (const void * a, const void * b) {
    if(*(uint32_t*)a == *(uint32_t*)b ) return 0;
    return *(uint32_t*)a < *(uint32_t*)b ? -1 : 1;
}

void PrintArrayU32(uint32_t *arr, int n){
    printf("(");
    for(int i = 0; i < n; ++i){
        printf("%u,", arr[i]);
    }
    printf(")\n");
}

void PrintAabb(Aabb *a){
    printf("p1[x:%u,y:%u] p2[x2:%u,y2:%u]\n", a->x, a->y, a->x2, a->y2);
}

int AreEqual(Aabb *a, Aabb *b){
    return (a->x == b->x && a->x2 == b->x2 && a->y == b->y && a->y2 == b->y2);
}

void PushIntersection(Aabb *inter);

int GenDifference2(Aabb *a, Aabb *subtract){
    if(Contains(subtract, a)) return 0;
    Aabb intersection = {0};
    if(!GetIntersection(a, subtract, &intersection)){
        return 0;
    }

    Aabb splits[4] = {0};
    
    Aabb *splitTop = NULL;
    Aabb *splitLeft = NULL;
    Aabb *splitBottom = NULL;
    Aabb *splitRight = NULL;

    //split top
    if(subtract->y > a->y){
        splitTop = splits;
        splitTop->x = a->x;
        splitTop->y = a->y;
        splitTop->y2 = subtract->y;
        splitTop->x2 = a->x2;
    }

    //split left
    if(subtract->x > a->x){
        splitLeft = splits + 1;
        splitLeft->x = a->x;
        splitLeft->y = (splitTop) ? splitTop->y2 : a->y;
        splitLeft->x2 = subtract->x;
        splitLeft->y2 = (subtract->y2 < a->y2) ? subtract->y2 : a->y2;
    }

    //split Bottom
    if(subtract->y2 < a->y2){
        splitBottom = splits + 2;
        splitBottom->x = a->x;
        splitBottom->y = subtract->y2;
        splitBottom->x2 = a->x2;
        splitBottom->y2 = a->y2;
    }

    //split right
    if(subtract->x2 < a->x2){
        splitRight = splits + 3;
        splitRight->x = subtract->x2;
        splitRight->y = (splitTop) ? splitTop->y2 : a->y;
        splitRight->x2 = a->x2;
        splitRight->y2 = (splitBottom) ? splitBottom->y : a->y2;
    }

    if(splitTop){
        PushIntersection(splitTop);
    }
    if(splitLeft){
        PushIntersection(splitLeft);
    }
    if(splitBottom){
        PushIntersection(splitBottom);
    }
    if(splitRight){
        PushIntersection(splitRight);
    }

    return 1;
}

int GenDifference(Aabb *a, Aabb *subtract){
    if(Contains(subtract, a)) return 0;
    Aabb intersection = {0};
    if(!GetIntersection(a, subtract, &intersection)){
        return 0;
    }
    // if(AreEqual(a, &intersection)){
    //     printf("No diff\n");
    //     return 1; //the subtraction consumes all of a, so we can discard it
    // }
    
    //printf("Subtracting:"); PrintAabb(subtract);
    uint32_t xlist[4] = {a->x, a->x2, UINT32_MAX, UINT32_MAX};
    uint32_t ylist[4] = {a->y, a->y2, UINT32_MAX, UINT32_MAX};
    int xCount = 2;
    int yCount = 2;

    if(subtract->x > a->x) xlist[++xCount - 1] = subtract->x;
    if(subtract->x2 < a->x2) xlist[++xCount -1] = subtract->x2;
    if(subtract->y > a->y) ylist[++yCount -1] = subtract->y;
    if(subtract->y2 < a->y2) ylist[++yCount -1] = subtract->y2;
    //sort coordinates
    qsort(xlist, xCount, sizeof(uint32_t), SortFunc);
    qsort(ylist, yCount, sizeof(uint32_t), SortFunc);
    //remove duplicate elements
    for(int i = 3; i > 1; --i){
        if(xlist[i] != UINT32_MAX && xlist[i] == xlist[i - 1]) {
            xlist[i] = UINT32_MAX;
            --xCount;
        }
        if(ylist[i] != UINT32_MAX && ylist[i] == ylist[i - 1]){
            ylist[i] = UINT32_MAX;  
            --yCount;
        } 
    }

    //printf("X's:"); PrintArrayU32(xlist, xCount);
   // printf("Y's:"); PrintArrayU32(ylist, yCount);

    Aabb splits[9] = {0};
    int splitCount = 0;

    for(int i = 0; i < xCount - 1; ++i){
        for(int j = 0; j < yCount - 1; ++j){
            Aabb *split = splits + splitCount++;
            split->x = xlist[i];
            split->y = ylist[j];
            split->x2 = xlist[i + 1];
            split->y2 = ylist[j + 1];
        }
    }

    //printf("%i splits:\n", splitCount);
    for(int i = 0; i < splitCount; ++i){
        if(AreEqual(splits + i, &intersection)) continue; //discard original intersection, we only want difference
        PushIntersection(splits + i);
    }

    return 1;
}

void PushIntersection(Aabb *inter){
    //printf("Pushing "); PrintAabb(inter);

    int genDiff = 0;
    for(int i = 0; i < intersectionCount; ++i){
        if(Contains(intersections +i, inter)) return;
        if(GenDifference2(inter, intersections + i)){
            genDiff = 1;
            //if we do get a difference, the splits will call this method recursively, so we are done checking with this 
            break;
        } 
    }
    //if we didnt split the rect, then it didnt collide with anything, add it to the global intersections
    if(!genDiff){
        intersections[intersectionCount++] = *inter;    
    }
    
}

int main(void){
    FILE *input = fopen("input.txt", "r");

    char line[32];
    while(fgets(line, 32, input)){
        if(boxCount >= MAX_BOXES){
            printf("Box limit reached.\n");
            return 1;
        }
        boxes[boxCount++] = ParseAabb(line);
    }
    fclose(input);

    //go through all boxes and get intersections
    Aabb intersection = {0};
    for(int i = 0; i < boxCount; ++i){
        Aabb *box1 = boxes + i;
        for(int j = i + 1; j < boxCount; ++j){
            Aabb *box2 = boxes + j;
            if(GetIntersection(box1, box2, &intersection)){
                PushIntersection(&intersection);
            }
                //TakeSpace(&intersection);
//                area += AabbArea(interBoxes + (interCount - 1));
        }
    }

    for(int i = 0; i < intersectionCount; ++i){
        areaSum += GetArea(intersections + i);
    }
    printf("total area %i inters = %i\n", areaSum, intersectionCount);

    // for(int i = 0; i < intersectionCount; ++i){
    //     Aabb *b1 = intersections + i;
    //     int area1 = AabbArea(b1);
    //     for(int j = i + 1; j < intersectionCount; ++j){
    //         Aabb *b2 = intersections + j;
    //         int area2 = AabbArea(b2);
    //         if(AreIntersecting(b1, b2)){
    //             intersection = GetIntersection(b1, b2);
    //             int iArea = AabbArea(&intersection);
    //             if(!IsFlag(b1) && !IsFlag(b2)){
    //                 areaSum += (area1 + area2) - iArea;
    //                 FlagAabb(b1);
    //                 FlagAabb(b2);
    //                 continue; 
    //             } else if(IsFlag(b1) && !IsFlag(b2)){
    //                 areaSum += area2 - iArea;
    //                 FlagAabb(b2);
    //                 continue;
    //             } else if(!IsFlag(b1) && IsFlag(b2)){
    //                 areaSum += area1 - iArea;
    //                 FlagAabb(b1);
    //                 continue;
    //             }
    //         }
    //     }
    //     if(!IsFlag(b1)){
    //         areaSum += area1;
    //     }
    // }



    //area += AabbArea(interBoxes + (interCount - 1));


    printf("%i", areaSum);

    
}