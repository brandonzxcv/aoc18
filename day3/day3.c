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

#define WIN_HEIGHT 1050

typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int x;
    int y;
    float scale;
    //RenderStepType renderStep;
    TTF_Font *mainFont;
} Visualizer;

void visual_init(void);
void visual_destroy(void);
void renderstep_load_aabb(void);
void renderstep_bv_split(void);
void renderstep_bv_insert_aabb(Aabb *bv, Aabb *insert, Aabb *intersection);
void renderstep_begin_bv_intersection_test(void);
void renderstep_bv_intersection_test(Aabb *inter);
void renderstep_finish_bv_intersection_test(void);
void renderstep_finish_bv_intersection_test_zoom_out(void);
void renderstep_calculate_areas(void);

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
};
    
static SDL_Color SdlColorWhite = {255,255,255,255};
static SDL_Color SdlColorBlack = {0,0,0,255};
static SDL_Color SdlColorRed = {255,0,0,255};
static SDL_Color SdlColorBlank = {0,0,0,0};

static Aabb *rsInterStepBox1;
static Aabb *rsInterStepBox2;
#endif

Aabb parse_aabb(char *line){
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

int aabb_are_intersecting(Aabb *box1, Aabb *box2){
    return !(box2->x >= box1->x2 || box1->x >= box2->x2 || box2->y >= box1->y2 || box1->y >= box2->y2);
}

int aabb_intersection(Aabb *a, Aabb *b, Aabb *result){
    if(!aabb_are_intersecting(a, b)) 
        return 0;

    result->x = (a->x < b->x) ? b->x : a->x;
    result->x2 = (a->x2 < b->x2) ? a->x2 : b->x2;
    result->y = (a->y < b->y) ? b->y : a->y;
    result->y2 = (a->y2 < b->y2) ? a->y2 : b->y2;

    return 1;
}

int aabb_contains(Aabb *a, Aabb *b){
    return (b->x >= a->x && b->x2 <= a->x2 && b->y >= a->y && b->y2 <= a->y2);
}

// int aabb_equal(Aabb *a, Aabb *b){
//     return (a->x == b->x && a->x2 == b->x2 && a->y == b->y && a->y2 == b->y2);
// }

int aabb_area(Aabb *a){
    return (a->x2 - a->x) * (a->y2 - a->y);
}

void bv_insert_intersection(Bv *bv, Aabb *inter);

//subtract an existing aabb from our new intersection a, and try to insert the new splits
int split_intersection_difference(Bv *bv, Aabb *a, Aabb *subtract){
    if(aabb_contains(subtract, a)) return 0;
    Aabb intersection = {0};
    if(!aabb_intersection(a, subtract, &intersection)){
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
        bv_insert_intersection(bv, splitTop);
    }
    if(splitLeft){
        bv_insert_intersection(bv, splitLeft);
    }
    if(splitBottom){
        bv_insert_intersection(bv, splitBottom);
    }
    if(splitRight){
        bv_insert_intersection(bv, splitRight);
    }

    return 1;
}

void bv_insert_intersection(Bv *bv, Aabb *inter){
    for(int i = 0; i < bv->interEntityCount; ++i){
        if(aabb_contains(bv->interEntities + i, inter)) 
            return;
        if(split_intersection_difference(bv, inter, bv->interEntities + i)){
            //if we do get a difference, the intersection has been split, and the splits have been added
            return;
        } 
    }
    //if we didnt split the aabb, then it didnt collide with anything -> add it
    bv->interEntities[bv->interEntityCount++] = *inter;
    ++bvInterEntityCount;
    #ifdef VISUAL
    if(bv - bvs == 5)
        renderstep_bv_intersection_test(inter);
    #endif
}

//split bounding volume into smaller leaf nodes
void bv_split(Bv *bv, int depth){
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
    renderstep_bv_split();
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
    renderstep_bv_split();
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
    renderstep_bv_split();
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
    renderstep_bv_split();
    #endif

    bv_split(topLeft, depth + 1);
    bv_split(topRight, depth + 1);
    bv_split(bottomLeft, depth + 1);
    bv_split(bottomRight, depth + 1);

    return;
}

void bv_insert_aabb(Bv *bv, Aabb *a){
    //no leaf nodes, insert into this bv
    if(!bv->q[0]){
        bv->entities[bv->entityCount++] = *a;
        ++bvEntityCount;
        return;
    }
    //find intersecting leaf nodes and insert the intersection aabb
    Aabb inter = {0};
    for(int i = 0; i < 4; ++i){
        if(aabb_intersection((Aabb*)bv->q[i], a, &inter)){
            #ifdef P2
            inter.id = a->id;
            #endif
            #ifdef VISUAL
            renderstep_bv_insert_aabb((Aabb*)bv->q[i], a, &inter);
            #endif
            bv_insert_aabb(bv->q[i], &inter);
        }
    }
}

int main(void){
    FILE *input = open_file("input_alex.txt", "r");

    #ifdef VISUAL
    visual_init();
    #endif
    
    bvRoot = bvs + bvCount++;
    bvRoot->x = bvRoot->y = UINT32_MAX;
    bvRoot->entities = bvEntities;
    bvRoot->interEntities = bvInterEntities;
    //read all aabb's into memory, and expand root bounding volume while doing so
    char line[32];
    while(fgets(line, 32, input)){
        if(boxCount >= MAX_BOXES){
            printf("Box limit reached.\n");
            return 1;
        }
        Aabb box = parse_aabb(line);
        bvRoot->x = MIN(box.x, bvRoot->x);
        bvRoot->y = MIN(box.y, bvRoot->y);
        bvRoot->x2 = MAX(box.x2, bvRoot->x2);
        bvRoot->y2 = MAX(box.y2, bvRoot->y2);
        boxes[boxCount++] = box;
        #ifdef VISUAL
        renderstep_load_aabb();
        #endif
    }
    fclose(input);

    //split bounding volume into smaller children volumes until max depth is reached
    bv_split(bvRoot, 1);

    printf("BV dimensions = [%u, %u, %u,%u], Total bvs = %u\n", bvRoot->x, bvRoot->y, bvRoot->x2, bvRoot->y2, bvCount);

    //insert the loaded aabb's into their coresponding bounding volumes, splitting them into smaller aabb's if necessary
    for(int i = 0; i < boxCount; ++i){
        Aabb *box = boxes + i;
        bv_insert_aabb(bvRoot, box);
    }

    #ifdef VISUAL
    renderstep_begin_bv_intersection_test();
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
                if(aabb_intersection(b1, b2, &intersection)){
                    #ifdef VISUAL
                    rsInterStepBox1 = b1;
                    rsInterStepBox2 = b2;
                    #endif
                    bv_insert_intersection(currBv, &intersection);
                    #ifdef P2 //part 2 hack, flag any intersecting aabb's
                    boxes[b1->id - 1].id = 1;
                    boxes[b2->id - 1].id = 1;
                    #endif
                }
            }
        }
        
        //we can sum the areas of the rects on the intersection layer to get the answer to Part 1
        for(int ei = 0; ei < currBv->interEntityCount; ++ei){
            areaSum += aabb_area(currBv->interEntities + ei);
        }
    }

    #ifdef VISUAL
    renderstep_finish_bv_intersection_test();
    #endif

    printf("Part1 total area = %i Intersections = %i\n", areaSum, bvInterEntityCount);

    #ifdef P2
    //find any aabb's that were not flagged during intersection tests -> that is our non intersecting id
    for(int i = 0; i < boxCount; ++i){
        if(boxes[i].id != 1){
            printf("Part2 ID = %u\n", boxes[i].id);
            break;
        }
        
    }
    #endif

    #ifdef VISUAL
    visual_destroy();
    #endif

    return 0;
}

#ifdef VISUAL
void visual_init(void){
    if(TTF_Init() == -1) {
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

    vis.window = SDL_CreateWindow("AoC Day 3 Visualization", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_HEIGHT, WIN_HEIGHT, 0);
    vis.renderer = SDL_CreateRenderer(vis.window, -1, 0);
    vis.x = 0;
    vis.y = 0;
    vis.scale = WIN_HEIGHT / 1050.0f;

    SDL_SetRenderDrawBlendMode(vis.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetScale(vis.renderer, vis.scale, vis.scale);
}

void visual_destroy(void){
    SDL_DestroyRenderer(vis.renderer);
    SDL_DestroyWindow(vis.window);
}

float ease(EaseType easeType, float startTime, float duration, float start, float end) {
    float t = (SDL_GetTicks() / 1000.0f) - startTime;
    if(t >= duration) return end;
    t = t / duration;
    switch(easeType){
        case EaseTypeInOutQuad: return Lerp(start, end, T_EaseInOutQuad(t));
        case EaseTypeInQuad: return Lerp(start, end, T_EaseInQuad(t));
    }
    
};

SDL_Color color_random(int i){
    int rgb = (int)floor(abs(sinf(i) * 16777215)) % 16777215;    
    return (SDL_Color) {
        rgb & 0xFF,
        (rgb >> 8) & 0xFF,
        (rgb >> 16) & 0xFF,
        255
    };
}

void render_box(Aabb *box, SDL_Color fill, SDL_Color outline){
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

void render_text_scaled(char *str, int x, int y, float scale){
    float currScale;
    SDL_RenderGetScale(vis.renderer, &currScale, &currScale);
    SDL_RenderSetScale(vis.renderer, scale, scale);
    SDL_Surface* textSurface = TTF_RenderText_Blended(vis.mainFont, str, SdlColorWhite);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(vis.renderer, textSurface);
    SDL_Rect renderQuad = { (vis.x + x) * currScale, (vis.y + y) * currScale, textSurface->w, textSurface->h };
    SDL_RenderCopy(vis.renderer, texture, NULL, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
    SDL_RenderSetScale(vis.renderer, currScale, currScale);
}

void renderstep_load_aabb(void){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    for(int i = 0; i < boxCount; ++i){
        render_box(boxes + i, SdlColorRed, SdlColorBlack);
    }

    static char entityCountTxt[40];
    snprintf(entityCountTxt, 40, "Loading Entities [%i]", boxCount);
    render_text_scaled(entityCountTxt, bvs[0].x, bvs[0].y2 + 10, 1);

    render_box((Aabb *)bvRoot, SdlColorBlank, SdlColorWhite);
    SDL_RenderPresent(vis.renderer);

    static int rsBoxLoadDrawDelay = 100;
    SDL_Delay(rsBoxLoadDrawDelay);
    if(rsBoxLoadDrawDelay > 1)
        rsBoxLoadDrawDelay -= 1;
}

void renderstep_bv_split(void){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    for(int i = 0; i < boxCount; ++i){
        render_box(boxes + i, (SDL_Color){150, 0, 0, 255}, SdlColorBlack);
    }

    for(int i = 1; i < bvCount; ++i){
        render_box(bvs + i, SdlColorBlank, SdlColorWhite);
    }

    static char quadTreeSplitTxt[40];
    snprintf(quadTreeSplitTxt, 40, "Creating quad tree leafs [%i]", bvCount);
    render_text_scaled(quadTreeSplitTxt, bvs[0].x, bvs[0].y2 + 10, 1);

    SDL_RenderPresent(vis.renderer);
    SDL_Delay(100);
}

void renderstep_bv_insert_aabb(Aabb *bv, Aabb *insert, Aabb *intersection){    
    static int flashesDrawn = 0;
    fadeBv:
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    //original non sorted entities background layer
    for(int i = 0; i < boxCount; ++i){
        render_box(boxes + i, (SDL_Color){25, 0, 0, 255}, SdlColorBlack);
    }

    //quad tree outlines
    for(int i = 1; i < bvCount; ++i){
        render_box(bvs + i, SdlColorBlank, SdlColorWhite);
    }

    render_box(insert, (SDL_Color){0, 0, 0, 255}, (SDL_Color){255, 0, 0, 255});
    //render_box(intersection, 255, 255, 255, 255);

    //quad tree entities
    for(int i = 0; i < bvCount; ++i){
        Bv *currBv = bvs + i;
        if(currBv->entityCount <= 0) continue;

        for(int ei = 0; ei < currBv->entityCount; ++ei){
            Aabb *ent = currBv->entities + ei;
            render_box(ent, bvColors[i], SdlColorBlack);
        }
    }
    
    static char spatialTxt[40];
    //quad tree section flash
    if(flashesDrawn < 80){
        static uint8_t bvAlpha = 255;
        int alphaChange = 2 + ((float)flashesDrawn/80.0f) * 10;
        while(bvAlpha - alphaChange > 0){
            bvAlpha -= alphaChange;
            render_box(bv, (SDL_Color){100, 100, 100, bvAlpha}, SdlColorBlank);

            snprintf(spatialTxt, 40, "Spatial indexes [%i]", bvEntityCount);
            render_text_scaled(spatialTxt, bvs[0].x, bvs[0].y2 + 10, 1);

            SDL_RenderPresent(vis.renderer);        
            SDL_Delay(1);
            goto fadeBv;
        }
        bvAlpha = 255;
    } else {        
        snprintf(spatialTxt, 40, "Spatial indexes >> [%i]", bvEntityCount);
        render_text_scaled(spatialTxt, bvs[0].x, bvs[0].y2 + 10, 1);
    }
    ++flashesDrawn;
    SDL_RenderPresent(vis.renderer);
}

void renderstep_begin_bv_intersection_test(void){   
    SDL_Delay(1000);

    float zoomStart = SDL_GetTicks() / 1000.0f;
    float zoom = vis.scale;
    float fadeOut = 255;
    float borderFadeOut = 100;
    while(zoom < vis.scale * 3.9f){
        zoom = ease(EaseTypeInOutQuad, zoomStart, 4.0f, vis.scale, vis.scale * 3.9f);
        fadeOut = ease(EaseTypeInOutQuad, zoomStart, 4.0f, 255, 0);
        borderFadeOut = ease(EaseTypeInOutQuad, zoomStart, 4.0f, 100, 0);
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        //top left quad
        Bv *bv = bvs + 5;

        //draw all bv entities, and fade them out as we zoom into our quad
        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            for(int ei = 0; ei < currBv->entityCount; ++ei){
                Aabb *ent = currBv->entities + ei;
                SDL_Color col = bvColors[i];
                if(i != 5){
                    col.a = fadeOut;    
                }
                render_box(ent, col, (SDL_Color) {0, 0, 0, 255});    
            }
        }

        //fade out rest of bv outlines
        for(int i = 1; i < bvCount; ++i){
            if(i != 5){
                render_box(bvs + i, SdlColorBlank, (SDL_Color){255,255,255, borderFadeOut});
            }
            
        }

        //outline the quad we are zooming into
        render_box(bv, SdlColorBlank, SdlColorRed);

        SDL_RenderPresent(vis.renderer);
        SDL_RenderSetScale(vis.renderer, zoom, zoom);
        SDL_Delay(4);
    }
}

void renderstep_bv_intersection_test(Aabb *inter){    
    SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
    SDL_RenderClear(vis.renderer);

    Bv *bv = bvs + 5;

    //entity aabb layer as bg
    for(int ei = 0; ei < bv->entityCount; ++ei){
        Aabb *ent = bv->entities + ei;
        SDL_Color col = SdlColorWhite;
        col. a = 100;
        render_box(ent, col, (SDL_Color) {0, 0, 0, 255});    
    }

    //two entities we are testing for intersection currently
    render_box(rsInterStepBox1, SdlColorBlank, SdlColorRed);
    render_box(rsInterStepBox2, SdlColorBlank, SdlColorRed);

    //the intersection aabb layer
    for(int ei = 0; ei < bv->interEntityCount; ++ei){
        Aabb *ent = bv->interEntities + ei;
        render_box(ent, color_random(ei), SdlColorBlank);    
    }

    //highlight the current intersection area
    render_box(inter, SdlColorRed, SdlColorBlank);

    //quad bv outline
    render_box(bv, SdlColorBlank, SdlColorRed);

    static char interTxt[50];
    snprintf(interTxt, 50, "Unique Intersections (No Overlaps) [%i]", bv->interEntityCount);
    render_text_scaled(interTxt, bv->x, bv->y2 + 2, 1);

    SDL_RenderPresent(vis.renderer);
    SDL_Delay(150);
}

void renderstep_finish_bv_intersection_test(void){
    SDL_Delay(1000);

    float alphaStart = SDL_GetTicks() / 1000.0f;
    float alpha = 100;
    while(alpha > 0){
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        alpha = ease(EaseTypeInOutQuad, alphaStart, 3.0f, 100, 0);
        Bv *bv = bvs + 5;

        //entity layer bg
        for(int ei = 0; ei < bv->entityCount; ++ei){
            Aabb *ent = bv->entities + ei;
            SDL_Color col = SdlColorWhite;
            col.a = alpha;
            render_box(ent, col, (SDL_Color) {0, 0, 0, 255});    
        }

        //intersection layer
        for(int ei = 0; ei < bv->interEntityCount; ++ei){
            Aabb *ent = bv->interEntities + ei;
            SDL_Color col = color_random(ei);
            col.a = 255;
            render_box(ent, col, (SDL_Color) {0, 0, 0, 0});    
        }

        render_box(bv, SdlColorBlank, SdlColorRed);

        static char interTxt[50];
        snprintf(interTxt, 50, "Unique Intersections (No Overlaps) [%i]", bv->interEntityCount);
        render_text_scaled(interTxt, bv->x, bv->y2 + 2, 1);

        SDL_RenderPresent(vis.renderer);
        SDL_Delay(10);
    }

    SDL_Delay(500);
    renderstep_finish_bv_intersection_test_zoom_out();
}

void renderstep_finish_bv_intersection_test_zoom_out(void){
    float zoomStart = SDL_GetTicks() / 1000.0f;
    float oldZoom;
    SDL_RenderGetScale(vis.renderer, &oldZoom, &oldZoom);
    float zoom = oldZoom;
    float fadeIn = 0;
    while(zoom > vis.scale){
        zoom = ease(EaseTypeInOutQuad, zoomStart, 4.0f, oldZoom, vis.scale);
        fadeIn = ease(EaseTypeInOutQuad, zoomStart, 4.0f, 0, 255);
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        for(int i = 1; i < bvCount; ++i){
            render_box(bvs + i, SdlColorBlank, (SDL_Color){255,255,255, fadeIn});
        }

        //fade back in other quad bv's and their entities
        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                Aabb *ent = currBv->interEntities + ei;
                SDL_Color col = color_random(ei);
                if(i != 5){
                    col.a = fadeIn;    
                }
                render_box(ent, col, (SDL_Color) {0, 0, 0, 0});    
            }
        }

        SDL_RenderPresent(vis.renderer);
        SDL_RenderSetScale(vis.renderer, zoom, zoom);
        SDL_Delay(10);
    }

    SDL_Delay(2000);

    renderstep_calculate_areas();
}

void renderstep_calculate_areas(void){
    int bvDrawn = 0;
    int eiDrawn = 0;

    while(1){
        SDL_SetRenderDrawColor(vis.renderer, 0, 0, 0, 255);
        SDL_RenderClear(vis.renderer);

        for(int i = 1; i < bvCount; ++i){
            render_box(bvs + i, SdlColorBlank, SdlColorWhite);
        }

        for(int i = 0; i < bvCount; ++i){
            Bv *currBv = bvs + i;
            if(currBv->entityCount <= 0) continue;

            if(i < bvDrawn){
                int bvArea = 0;
                //sum area in quad
                for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                    Aabb *ent = currBv->interEntities + ei;
                    bvArea += aabb_area(ent);
                }
                int sides = sqrt(bvArea);
                //draw box in quad representing total area
                Aabb areaBox = {
                    currBv->x, 
                    currBv->y,
                    currBv->x + sides,
                    currBv->y + sides
                };

                render_box(&areaBox, bvColors[i], SdlColorBlank);

                static char areaTxt[20];
                snprintf(areaTxt, 20, "%i sq in", bvArea);
                render_text_scaled(areaTxt, currBv->x + 10, currBv->y2 - 40, 1);
                continue;
            }

            for(int ei = 0; ei < currBv->interEntityCount; ++ei){
                if(bvDrawn == 0) bvDrawn = i;

                Aabb *ent = currBv->interEntities + ei;
                SDL_Color col = color_random(ei);
                if(i <= bvDrawn && ei <= eiDrawn){
                    int area = aabb_area(ent);
                    ent->x = currBv->x;
                    ent->y = currBv->y + ei;
                    ent->x2 = ent->x + area;
                    ent->y2 = ent->y + 1;    
                }
                
                render_box(ent, col, SdlColorBlank);    
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
    render_text_scaled(totalTxt, bvs[0].x, bvs[0].y2 + 10, 1);
    SDL_RenderPresent(vis.renderer);

    SDL_Delay(20000);
}

#endif