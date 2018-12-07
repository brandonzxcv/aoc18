#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include "../include/br.h"

//#define P2

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t x2;
    uint32_t y2;
    #ifdef P2
    uint32_t id;
    #endif
} Aabb;

struct Bv {
    uint32_t x;
    uint32_t y;
    uint32_t x2;
    uint32_t y2;    
    struct Bv *q[4];
    Aabb *entities;
    int entityCount;
    Aabb *interEntities;
    int interEntityCount; 
};
typedef struct Bv Bv;

#define MAX_BOXES 4096

static Aabb boxes[MAX_BOXES];
static int boxCount;

static Aabb bvEntities[MAX_BOXES];
static int bvEntityCount;

static Aabb bvInterEntities[MAX_BOXES];
static int bvInterEntityCount;

#define BV_DEPTH 2
static Bv bvs[1 + 4 + 16];
static int bvCount;

static int areaSum;

Aabb ParseAabb(char *line){
    Aabb result = {0};
    #ifdef P2
    result.id = atoi(line + 1);
    #endif
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

int AreEqual(Aabb *a, Aabb *b){
    return (a->x == b->x && a->x2 == b->x2 && a->y == b->y && a->y2 == b->y2);
}

int GetArea(Aabb *a){
    return (a->x2 - a->x) * (a->y2 - a->y);
}

void PrintAabb(Aabb *a){
    printf("p1[x:%u,y:%u] p2[x2:%u,y2:%u]\n", a->x, a->y, a->x2, a->y2);
}

void BvPushIntersection(Bv *bv, Aabb *inter);

int GenDifference(Bv *bv, Aabb *a, Aabb *subtract){
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
        BvPushIntersection(bv, splitTop);
    }
    if(splitLeft){
        BvPushIntersection(bv, splitLeft);
    }
    if(splitBottom){
        BvPushIntersection(bv, splitBottom);
    }
    if(splitRight){
        BvPushIntersection(bv, splitRight);
    }

    return 1;
}

void BvPushIntersection(Bv *bv, Aabb *inter){
    int genDiff = 0;
    for(int i = 0; i < bv->interEntityCount; ++i){
        if(Contains(bv->interEntities + i, inter)) return;
        if(GenDifference(bv, inter, bv->interEntities + i)){
            genDiff = 1;
            //if we do get a difference, the intersection has been split, and the splits have been added
            break;
        } 
    }
    //if we didnt split the rect, then it didnt collide with anything
    if(!genDiff){
        bv->interEntities[bv->interEntityCount++] = *inter;
        ++bvInterEntityCount;
    }    
}

void SplitBv(Bv *bv, int depth){
    if(depth > BV_DEPTH) return;

    uint32_t childWidth = (bv->x2 - bv->x) / 2;
    uint32_t childHeight = (bv->y2 - bv->y) / 2;
    uint32_t childMaxBoxes = (MAX_BOXES / (int)pow(4, depth));

    //top left
    Bv *topLeft = bv->q[0] = bvs + bvCount++;
    topLeft->x = bv->x;
    topLeft->y = bv->y;
    topLeft->x2 = bv->x + childWidth;
    topLeft->y2 = bv->y + childHeight;
    topLeft->entities = bv->entities;
    topLeft->interEntities = bv->interEntities;
    //top right
    Bv *topRight = bv->q[1] = bvs + bvCount++;
    topRight->x = topLeft->x2;
    topRight->y = topLeft->y;
    topRight->x2 = bv->x2;
    topRight->y2 = topLeft->y2;
    topRight->entities = bv->entities + childMaxBoxes;
    topRight->interEntities = bv->interEntities + childMaxBoxes;

    //bottom left
    Bv *bottomLeft = bv->q[2] = bvs + bvCount++;
    bottomLeft->x = topLeft->x;
    bottomLeft->y = topLeft->y2;
    bottomLeft->x2 = topLeft->x2;
    bottomLeft->y2 = bv->y2;
    bottomLeft->entities = bv->entities + childMaxBoxes * 2;
    bottomLeft->interEntities = bv->interEntities + childMaxBoxes * 2;
    
    //bottom right
    Bv *bottomRight = bv->q[3] = bvs + bvCount++;
    bottomRight->x = topRight->x;
    bottomRight->y = topRight->y2;
    bottomRight->x2 = topRight->x2;
    bottomRight->y2 = bv->y2;
    bottomRight->entities = bv->entities + childMaxBoxes * 3;
    bottomRight->interEntities = bv->interEntities + childMaxBoxes * 3;

    SplitBv(topLeft, depth + 1);
    SplitBv(topRight, depth + 1);
    SplitBv(bottomLeft, depth + 1);
    SplitBv(bottomRight, depth + 1);

    return;
}

void InsertIntoBV(Bv *bv, Aabb *a){
    if(!bv->q[0]){
        bv->entities[bv->entityCount++] = *a;
        //printf("inserted %i\n", bv->entityCount);
        ++bvEntityCount;
        return;
    }
    Aabb inter = {0};
    for(int i = 0; i < 4; ++i){
        if(GetIntersection((Aabb*)bv->q[i], a, &inter)){
            #ifdef P2
            inter.id = a->id;
            #endif
            InsertIntoBV(bv->q[i], &inter);
        }
    }
}

int main(int argc, char **argv){
    FILE *input = fopen("input.txt", "r");

    //read all rects in to memory, and expand root bounding volume while doing so
    char line[32];
    Bv *bv = bvs + bvCount++;
    bv->entities = bvEntities;
    bv->interEntities = bvInterEntities;
    while(fgets(line, 32, input)){
        if(boxCount >= MAX_BOXES){
            printf("Box limit reached.\n");
            return 1;
        }
        Aabb box = ParseAabb(line);
        bv->x2 = MAX(box.x2, bv->x2);
        bv->y2 = MAX(box.y2, bv->y2);
        boxes[boxCount++] = box;
    }
    fclose(input);

    //split bounding volume into smaller children volumes until max depth is reached
    SplitBv(bv, 1);

    printf("BV dimensions = [%u,%u], Total bvs = %u\n", bv->x2, bv->y2, bvCount);

    //insert the rects into their coresponding bounding volumes, splitting them into smaller rects if necessary
    for(int i = 0; i < boxCount; ++i){
        Aabb *box1 = boxes + i;
        InsertIntoBV(bv, box1);
        // printf("i: %i entities: %u\n", i, bvEntityCount);
    }

    //for every bounding volume that contains entities, do an intersection test with the other entities within the volume, and store the intersections in a separate layer
    //the intersections will be tested separately amongst themselves and we will subtract the differences between them, in order to end up with no overlapping intersections
    for(int i = 0; i < bvCount; ++i){
        Bv *currBv = bvs + i;
        if(currBv->entityCount <= 0) continue;

        Aabb intersection = {0};

        for(int ei = 0; ei < currBv->entityCount; ++ei){
            Aabb *b1 = currBv->entities + ei;
            for(int j = ei + 1; j < currBv->entityCount; ++j){
                Aabb *b2 = currBv->entities + j;
                if(GetIntersection(b1, b2, &intersection)){
                    BvPushIntersection(currBv, &intersection);
                    #ifdef P2 //part 2 hack
                    boxes[b1->id - 1].id = 1;
                    boxes[b2->id - 1].id = 1;
                    #endif
                }
            }
        }
        
        //we can sum the areas of the rects on the intersection layer to get the answer to Part 1
        for(int ei = 0; ei < currBv->interEntityCount; ++ei){
            areaSum += GetArea(currBv->interEntities + ei);
        }
    }

    printf("Part1 total area = %i Intersections = %i\n", areaSum, bvInterEntityCount);

    #ifdef P2
    for(int i = 0; i < boxCount; ++i){
        if(boxes[i].id != 1){
            printf("Part2 ID = %u\n", boxes[i].id);
            break;
        }
        
    }
    #endif
}