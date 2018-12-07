#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../include/br.h"

//#define P2
#define VISUAL


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

static Bv *bvRoot;

static int areaSum;

#ifdef VISUAL
#include "SDL_ttf.h"

// typedef enum{
//     RsBoxLoad,
//     RsSplitBoundingVolume,
//     RsInsertIntoBv
// } RenderStepType;


typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int x;
    int y;
    //RenderStepType renderStep;
    TTF_Font *mainFont;
} Visualizer;

void InitVisual(void);
void DestroyVisual(void);
void RenderStepBoxLoad(void);
void RenderStepSplitBv(void);
void RenderStepInsertIntoBV(Aabb *bv, Aabb *insert, Aabb *intersection);
void RenderStepTransitionToInterTest(void);
void RenderStepInterTest(Aabb *inter);
void RenderStepInterSteprResultsTransition(void);


static Visualizer vis;


static SDL_Color bvColors[30] = {
    {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0x9b, 0xcd, 0xff, 0xFF},//light blue
        {0xFF, 0xFF, 0x00, 0xFF},//yellow
        {0xAA, 0x10, 0x10, 0xFF},//red
        {0x0, 0xFF, 0x00, 0xFF},//green
        

        {0xc1,0x9b,0xff,0xff},//purple
        {0xF0,0x32,0xE6,0xFF},//magenta
        {0xff,0x98,0x11,0xff},//orange
        {0xbb,0xff,0x11,0xff},//greenish

        {0xff,0xd4,0x00,0xff},//yellowish
        {0x77,0x42,0xff,0xff},
        {0x42,0xe2,0xff,0xff},
        {0xff,0x88,0x00,0xff},//orangeish
        
        {0x0, 0x72,0xff,0xff},//blue
        {0xa0,0x0,0x45,0xff},//red
        {0x6e,0xff,0,0xff},//green
        {0x80,0x80,0,0xff},//olive
        //{0xE6, 0xE3, 0xB3, 0xFF}
//     {0x66, 0x80, 0xB3},
//     {0x66, 0x99, 0x1A}, 
//     {0xFF, 0x99, 0xE6}
};
    
static SDL_Color SdlColorWhite = {255,255,255,255};
static SDL_Color SdlColorRed = {255,0,0,255};

static Aabb *rsInterStepBox1;
static Aabb *rsInterStepBox2;
#endif

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
    for(int i = 0; i < bv->interEntityCount; ++i){
        if(Contains(bv->interEntities + i, inter)) return;
        if(GenDifference(bv, inter, bv->interEntities + i)){
            //if we do get a difference, the intersection has been split, and the splits have been added
            return;
        } 
    }
    //if we didnt split the rect, then it didnt collide with anything
    bv->interEntities[bv->interEntityCount++] = *inter;
    ++bvInterEntityCount;
    #ifdef VISUAL
    if(bv - bvs == 5)
        RenderStepInterTest(inter);
    #endif
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
    #ifdef VISUAL
    RenderStepSplitBv();
    #endif

    //top right
    Bv *topRight = bv->q[1] = bvs + bvCount++;
    topRight->x = topLeft->x2;
    topRight->y = topLeft->y;
    topRight->x2 = bv->x2;
    topRight->y2 = topLeft->y2;
    topRight->entities = bv->entities + childMaxBoxes;
    topRight->interEntities = bv->interEntities + childMaxBoxes;
    #ifdef VISUAL
    RenderStepSplitBv();
    #endif


    //bottom left
    Bv *bottomLeft = bv->q[2] = bvs + bvCount++;
    bottomLeft->x = topLeft->x;
    bottomLeft->y = topLeft->y2;
    bottomLeft->x2 = topLeft->x2;
    bottomLeft->y2 = bv->y2;
    bottomLeft->entities = bv->entities + childMaxBoxes * 2;
    bottomLeft->interEntities = bv->interEntities + childMaxBoxes * 2;
    #ifdef VISUAL
    RenderStepSplitBv();
    #endif

    
    //bottom right
    Bv *bottomRight = bv->q[3] = bvs + bvCount++;
    bottomRight->x = topRight->x;
    bottomRight->y = topRight->y2;
    bottomRight->x2 = topRight->x2;
    bottomRight->y2 = bv->y2;
    bottomRight->entities = bv->entities + childMaxBoxes * 3;
    bottomRight->interEntities = bv->interEntities + childMaxBoxes * 3;
    #ifdef VISUAL
    RenderStepSplitBv();
    #endif

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
            #ifdef VISUAL
            RenderStepInsertIntoBV((Aabb*)bv->q[i], a, &inter);
            #endif
            InsertIntoBV(bv->q[i], &inter);
        }
    }
}

int main(int argc, char **argv){
    #ifdef VISUAL
    InitVisual();
    #endif
    //read all rects in to memory, and expand root bounding volume while doing so
    FILE *input = fopen("input_alex.txt", "r");
    char line[32];
    bvRoot = bvs + bvCount++;
    bvRoot->x = bvRoot->y = UINT32_MAX;
    bvRoot->entities = bvEntities;
    bvRoot->interEntities = bvInterEntities;
    while(fgets(line, 32, input)){
        if(boxCount >= MAX_BOXES){
            printf("Box limit reached.\n");
            return 1;
        }
        Aabb box = ParseAabb(line);
        bvRoot->x = MIN(box.x, bvRoot->x);
        bvRoot->y = MIN(box.y, bvRoot->y);
        bvRoot->x2 = MAX(box.x2, bvRoot->x2);
        bvRoot->y2 = MAX(box.y2, bvRoot->y2);
        boxes[boxCount++] = box;
        #ifdef VISUAL
        RenderStepBoxLoad();
        #endif
    }
    fclose(input);

    //split bounding volume into smaller children volumes until max depth is reached
    SplitBv(bvRoot, 1);

    printf("BV dimensions = [%u, %u, %u,%u], Total bvs = %u\n", bvRoot->x, bvRoot->y, bvRoot->x2, bvRoot->y2, bvCount);

    //insert the rects into their coresponding bounding volumes, splitting them into smaller rects if necessary
    for(int i = 0; i < boxCount; ++i){
        Aabb *box1 = boxes + i;
        InsertIntoBV(bvRoot, box1);
        // printf("i: %i entities: %u\n", i, bvEntityCount);
    }
    #ifdef VISUAL
    RenderStepTransitionToInterTest();
    #endif
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
                    #ifdef VISUAL
                    rsInterStepBox1 = b1;
                    rsInterStepBox2 = b2;
                    #endif
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
    #ifdef VISUAL
    RenderStepInterSteprResultsTransition();
    #endif

    printf("Part1 total area = %i Intersections = %i\n", areaSum, bvInterEntityCount);

    #ifdef P2
    for(int i = 0; i < boxCount; ++i){
        if(boxes[i].id != 1){
            printf("Part2 ID = %u\n", boxes[i].id);
            break;
        }
        
    }
    #endif

    #ifdef VISUAL
    DestroyVisual();
    #endif

    return 0;
}

#ifdef VISUAL
void InitVisual(void){
    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }
    vis.mainFont = TTF_OpenFont("calibri.ttf", 40);
    if(!vis.mainFont) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(2);
    }
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    vis.window = SDL_CreateWindow("AoC Day 3 Visualization", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1380, 1380, 0);
    vis.renderer = SDL_CreateRenderer(vis.window, -1, 0);
    vis.x = 10;
    vis.y = 10;

    SDL_SetRenderDrawBlendMode(vis.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetScale(vis.renderer, 1.3f, 1.3f);
}

void DestroyVisual(void){
    SDL_DestroyRenderer(vis.renderer);
    SDL_DestroyWindow(vis.window);
}

typedef enum{
    EaseTypeInOutQuad,
    EaseTypeInQuad
} EaseType;

float T_EaseInQuad(float t){
    return t*t;
}
float T_EaseInOutQuad(float t) {
    return (t < 0.5f) ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

float Lerp(float start, float end, float t){
    return start + (end - start) * t;
}

float Ease(EaseType easeType, float startTime, float duration, float start, float end) {
    float t = (SDL_GetTicks() / 1000.0f) - startTime;
    if(t >= duration) return end;
    t = t / duration;
    switch(easeType){
        case EaseTypeInOutQuad: return Lerp(start, end, T_EaseInOutQuad(t));
        case EaseTypeInQuad: return Lerp(start, end, T_EaseInQuad(t));
    }
    
};

SDL_Color RandomColor(int i){
    int rgb = (int)floor(abs(sinf(i) * 16777215)) % 16777215;    
    return (SDL_Color) {
        rgb & 0xFF,
        (rgb >> 8) & 0xFF,
        (rgb >> 16) & 0xFF,
        255
    };
}

void RenderBoxEx(Aabb *box, SDL_Color fill, SDL_Color outline){
    SDL_Rect rect = {vis.x + box->x, vis.y + box->y, box->x2 - box->x, box->y2 - box->y};
    if(fill.a > 0){
        SDL_SetRenderDrawColor(vis.renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect(vis.renderer, &rect);    
    }
    if(outline.a > 0){
        SDL_SetRenderDrawColor(vis.renderer, outline.r, outline.g, outline.b, outline.a);
        SDL_RenderDrawRect(vis.renderer, &rect);    
    }
}

void RenderBox(Aabb *box, uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    SDL_SetRenderDrawColor(vis.renderer, r, g, b, a);
    SDL_Rect rect = {vis.x + box->x, vis.y + box->y, box->x2 - box->x, box->y2 - box->y};
    SDL_RenderFillRect(vis.renderer, &rect);
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(vis.renderer, &rect);
}

void RenderBoundingVolumeRoot(void){
    SDL_SetRenderDrawColor(vis.renderer, 255, 255, 255, 255);
    SDL_Rect rect = {vis.x + bvRoot->x, vis.y + bvRoot->y, bvRoot->x2 - bvRoot->x, bvRoot->y2 - bvRoot->y};
    SDL_RenderDrawRect(vis.renderer, &rect);
}

void RenderBvChildOutline(Bv *bv, SDL_Color outline){
    SDL_Rect rect = {vis.x + bv->x, vis.y + bv->y, bv->x2 - bv->x, bv->y2 - bv->y};
    // if(withbg){
    //     SDL_SetRenderDrawColor(vis.renderer, 250, 50, 50, 25);    
    //     SDL_RenderFillRect(vis.renderer, &rect);
    // }
    SDL_SetRenderDrawColor(vis.renderer, outline.r, outline.g, outline.b, outline.a);
    SDL_RenderDrawRect(vis.renderer, &rect);
}

void RenderTextScaled(char *str, int x, int y, float scale){
    float oldScale;
    SDL_RenderGetScale(vis.renderer, &oldScale, &oldScale);
    SDL_RenderSetScale(vis.renderer, scale, scale);
    SDL_Surface* textSurface = TTF_RenderText_Blended(vis.mainFont, str, SdlColorWhite);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(vis.renderer, textSurface);
    SDL_Rect renderQuad = { (vis.x + x) * oldScale, (vis.y + y) * oldScale, textSurface->w, textSurface->h };
    SDL_RenderCopy(vis.renderer, texture, NULL, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
    SDL_RenderSetScale(vis.renderer, oldScale, oldScale);
}

void RenderStepBoxLoad(void){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    for(int i = 0; i < boxCount; ++i){
        RenderBox(boxes + i, 255, 0, 0, 255);
    }

    static char entityCountTxt[40];
    snprintf(entityCountTxt, 40, "Loading Entities [%i]", boxCount);
    RenderTextScaled(entityCountTxt, bvs[0].x, bvs[0].y2 + 10, 1);

    RenderBoundingVolumeRoot();

    SDL_RenderPresent(vis.renderer);

    static int rsBoxLoadDrawDelay = 100;
    SDL_Delay(rsBoxLoadDrawDelay);
    if(rsBoxLoadDrawDelay > 1) rsBoxLoadDrawDelay -= 1;

    
}

void RenderStepSplitBv(void){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);


    for(int i = 0; i < boxCount; ++i){
        RenderBox(boxes + i, 150, 0, 0, 255);
    }

    for(int i = 1; i < bvCount; ++i){
        RenderBvChildOutline(bvs + i, SdlColorWhite);
    }

    static char quadTreeSplitTxt[40];
    snprintf(quadTreeSplitTxt, 40, "Creating quad tree leafs [%i]", bvCount);
    RenderTextScaled(quadTreeSplitTxt, bvs[0].x, bvs[0].y2 + 10, 1);

    SDL_RenderPresent(vis.renderer);
    SDL_Delay(100);
}


void RenderStepInsertIntoBV(Aabb *bv, Aabb *insert, Aabb *intersection){    
    static int timesDrawn = 0;
    fadeBv:
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    //original non sorted entities background layer
    for(int i = 0; i < boxCount; ++i){
        RenderBox(boxes + i, 25, 0, 0, 255);
    }

    //quad tree outlines
    for(int i = 1; i < bvCount; ++i){
        RenderBvChildOutline(bvs + i, SdlColorWhite);
    }

    RenderBoxEx(insert, (SDL_Color){0, 0, 0, 255}, (SDL_Color){255, 0, 0, 255});
    //RenderBox(intersection, 255, 255, 255, 255);

    //quad tree entities
    for(int i = 0; i < bvCount; ++i){
        Bv *currBv = bvs + i;
        if(currBv->entityCount <= 0) continue;

        for(int ei = 0; ei < currBv->entityCount; ++ei){
            Aabb *ent = currBv->entities + ei;
            RenderBox(ent, bvColors[i].r, bvColors[i].g, bvColors[i].b, 255);
        }
    }


    
    static char spatialTxt[40];
    if(timesDrawn < 80){
        static uint8_t bvAlpha = 255;
        int alphaChange = 2 + ((float)timesDrawn/80.0f) * 10;
        while(bvAlpha - alphaChange > 0){
            bvAlpha -= alphaChange;
            SDL_SetRenderDrawColor(vis.renderer, 100, 100, 100, bvAlpha);
            SDL_Rect rect = {vis.x + bv->x, vis.y + bv->y, bv->x2 - bv->x, bv->y2 - bv->y};
            SDL_RenderFillRect(vis.renderer, &rect); 

            
            snprintf(spatialTxt, 40, "Spatial indexes [%i]", bvEntityCount);
            RenderTextScaled(spatialTxt, bvs[0].x, bvs[0].y2 + 10, 1);

            SDL_RenderPresent(vis.renderer);        
            SDL_Delay(1);
            goto fadeBv;
        }
        bvAlpha = 255;
    } else {        
        snprintf(spatialTxt, 40, "Spatial indexes >> [%i]", bvEntityCount);
        RenderTextScaled(spatialTxt, bvs[0].x, bvs[0].y2 + 10, 1);
    }
    ++timesDrawn;


    SDL_RenderPresent(vis.renderer);

}


void RenderStepTransitionToInterTest(void){   
    SDL_Delay(1000);

    float zoomStart = SDL_GetTicks() / 1000.0f;
    float zoom = 1.3f;
    float fadeOut = 255;
    float borderFadeOut = 100;
    while(zoom < 5.0f){
        zoom = Ease(EaseTypeInOutQuad, zoomStart, 4.0f, 1.3f, 5.0f);
        fadeOut = Ease(EaseTypeInOutQuad, zoomStart, 4.0f, 255, 0);
        borderFadeOut = Ease(EaseTypeInOutQuad, zoomStart, 4.0f, 100, 0);
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        Bv *bv = bvs + 5;

        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            for(int ei = 0; ei < currBv->entityCount; ++ei){
                Aabb *ent = currBv->entities + ei;
                SDL_Color col = bvColors[i];
                if(i != 5){
                    col.a = fadeOut;    
                }
                RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 255});    
            }
        }

        for(int i = 1; i < bvCount; ++i){
            if(i != 5){
                RenderBvChildOutline(bvs + i, (SDL_Color){255,255,255, borderFadeOut});
            }
            
        }

        //outline the quad we are zooming into
        RenderBvChildOutline(bv, SdlColorRed);

        SDL_RenderPresent(vis.renderer);
        SDL_RenderSetScale(vis.renderer, zoom, zoom);
        SDL_Delay(16);
    }
   
}


void RenderStepInterTest(Aabb *inter){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    Bv *bv = bvs + 5;

    //entity layer bg
    for(int ei = 0; ei < bv->entityCount; ++ei){
        Aabb *ent = bv->entities + ei;
        SDL_Color col = SdlColorWhite;
        col. a = 100;
        RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 255});    
    }

    RenderBoxEx(rsInterStepBox1, (SDL_Color){0, 0, 0, 0}, (SDL_Color){255, 0, 0, 255});
    RenderBoxEx(rsInterStepBox2, (SDL_Color){0, 0, 0, 0}, (SDL_Color){255, 0, 0, 255});

    //intersection layer
    for(int ei = 0; ei < bv->interEntityCount; ++ei){
        Aabb *ent = bv->interEntities + ei;
        RenderBoxEx(ent, RandomColor(ei)/*(SDL_Color){150, 50, 90, 255}*/, (SDL_Color) {0, 0, 0, 0});    
    }

    RenderBoxEx(inter, (SDL_Color){255, 0, 0, 255}, (SDL_Color){0, 0, 0, 0});

    RenderBvChildOutline(bv, SdlColorRed);

    static char interTxt[50];
    snprintf(interTxt, 50, "Unique Intersections (No Overlaps) [%i]", bv->interEntityCount);
    RenderTextScaled(interTxt, bv->x, bv->y2 + 2, 1);

    SDL_RenderPresent(vis.renderer);
    SDL_Delay(200);
}


void RenderStepInterSteprResultsTransition(void){

    SDL_Delay(1000);

    float alphaStart = SDL_GetTicks() / 1000.0f;
    float alpha = 100;
    while(alpha > 0){
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        alpha = Ease(EaseTypeInOutQuad, alphaStart, 3.0f, 100, 0);
        Bv *bv = bvs + 5;

        //entity layer bg
        for(int ei = 0; ei < bv->entityCount; ++ei){
            Aabb *ent = bv->entities + ei;
            SDL_Color col = SdlColorWhite;
            col.a = alpha;
            RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 255});    
        }

        //intersection layer
        for(int ei = 0; ei < bv->interEntityCount; ++ei){
            Aabb *ent = bv->interEntities + ei;
            SDL_Color col = RandomColor(ei);
            col.a = 255;//275 - alpha;
            RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 0});    
        }

        RenderBvChildOutline(bv, SdlColorRed);

        static char interTxt[50];
        snprintf(interTxt, 50, "Unique Intersections (No Overlaps) [%i]", bv->interEntityCount);
        RenderTextScaled(interTxt, bv->x, bv->y2 + 2, 1);

        SDL_RenderPresent(vis.renderer);
        SDL_Delay(10);
    }

    SDL_Delay(500);

    float zoomStart = SDL_GetTicks() / 1000.0f;
    float zoom = 5.0f;
    float fadeIn = 0;
    while(zoom > 1.3f){
        zoom = Ease(EaseTypeInOutQuad, zoomStart, 4.0f, 5.0f, 1.3f);
        fadeIn = Ease(EaseTypeInOutQuad, zoomStart, 4.0f, 0, 255);
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        for(int i = 1; i < bvCount; ++i){
            RenderBvChildOutline(bvs + i, (SDL_Color){255,255,255, fadeIn});
        }

        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                Aabb *ent = currBv->interEntities + ei;
                SDL_Color col = RandomColor(ei);
                if(i != 5){
                    col.a = fadeIn;    
                }
                RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 0});    
            }
        }

        SDL_RenderPresent(vis.renderer);
        SDL_RenderSetScale(vis.renderer, zoom, zoom);
        SDL_Delay(10);
    }

    SDL_Delay(2000);


    int bvDrawn = 0;
    int eiDrawn = 0;

    while(1){
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        for(int i = 1; i < bvCount; ++i){
            RenderBvChildOutline(bvs + i, (SDL_Color){255,255,255, 255});
        }

        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            if(i < bvDrawn){
                int bvArea = 0;
                //sum area in quad
                for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                    Aabb *ent = currBv->interEntities + ei;
                    bvArea += GetArea(ent);
                }
                int sides = sqrt(bvArea);
                //fill rect in quad representing total area
                SDL_Rect r = {
                    vis.x + currBv->x, vis.y + currBv->y,
                    sides, sides
                };
                SDL_Color col = bvColors[i];
                SDL_SetRenderDrawColor(vis.renderer, col.r, col.g, col.b, col.a);
                SDL_RenderFillRect(vis.renderer, &r);
                static char areaTxt[20];
                snprintf(areaTxt, 20, "%i sq in", bvArea);
                RenderTextScaled(areaTxt, currBv->x + 10, currBv->y2 - 30, 1);
                continue;
            }

            for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                if(bvDrawn == 0) bvDrawn = i;

                Aabb *ent = currBv->interEntities + ei;
                SDL_Color col = RandomColor(ei);
                if(i <= bvDrawn && ei <= eiDrawn){
                    int area = GetArea(ent);
                    ent->x = currBv->x;
                    ent->y = currBv->y + ei;
                    ent->x2 = ent->x + area;
                    ent->y2 = ent->y + 1;    
                }
                
                RenderBoxEx(ent, col, (SDL_Color) {0, 0, 0, 0});    
            }
            if(i == bvDrawn){
                ++eiDrawn;
                if(eiDrawn >= currBv->interEntityCount){
                    ++bvDrawn;
                    eiDrawn = 0;
                }
            }
            
        }

        SDL_RenderPresent(vis.renderer);
        SDL_Delay(1);

        if(bvDrawn == bvCount){
            ++bvDrawn;
            continue;
        }
        if(bvDrawn > bvCount){

            break;
        }
    }

    static char totalTxt[40];
    snprintf(totalTxt, 40, "Overlapping Area = %i sq in", areaSum);
    RenderTextScaled(totalTxt, bvs[0].x, bvs[0].y2 + 10, 1);
    SDL_RenderPresent(vis.renderer);

    SDL_Delay(20000);
}

#endif