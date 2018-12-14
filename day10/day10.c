#include "../include/br.h"
#include "SDL.h"
#include <string.h>

typedef struct{
    int x;
    int y;
    int vx;
    int vy;
} Light;

#define MAX_LIGHTS 1000
static Light lights[MAX_LIGHTS];
static int lightCount;
static int seconds;

void parse_light(char *line){
    Light *light = lights + lightCount++;
    line = strchr(line, '<') + 1;
    light->x = atoi(line);
    line = strchr(line, ',') + 1;
    light->y = atoi(line);
    line = strchr(line, '<') + 1;
    light->vx = atoi(line);
    line = strchr(line, ',') + 1;
    light->vy = atoi(line);
}

void move_lights_into_viewable_space(void){
    int allViewable = 0;
    while(!allViewable){
        allViewable = 1;
        for(int i = 0; i < lightCount; ++i){
            lights[i].x += lights[i].vx;
            lights[i].y += lights[i].vy;
            if(lights[i].x < 0 && lights[i].y < 0){
                allViewable = 0;
            }
        }   
        ++seconds; 
    }
}

void sim_lights(void){
    for(int i = 0; i < lightCount; ++i){
        lights[i].x += lights[i].vx;
        lights[i].y += lights[i].vy;
    }  
}

void draw_lights(SDL_Renderer *r){
    SDL_SetRenderDrawColor(r, 255,0,0,255);
    for(int i = 0; i < lightCount; ++i){
        SDL_RenderDrawPoint(r, lights[i].x, lights[i].y);
    } 
}

int main(int argc, char** argv){
    FILE *input = file_open("input.txt", "r");
    char line[64];
    while(fgets(line, 64, input)){
        parse_light(line);
    }
    fclose(input);

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
    SDL_Window *window = SDL_CreateWindow("Day 10", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    move_lights_into_viewable_space();

    int isRunning = 1;    
    while(isRunning){
        int nextStep = 0;
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            switch(e.type){
                case SDL_QUIT:
                isRunning = 0;
                break;
                case SDL_KEYUP:
                if(e.key.keysym.sym == SDLK_q){
                    nextStep = 1;
                }
                break;
            }
        }

        if(nextStep){
            sim_lights();
            ++seconds;
            printf("seconds = %i\n", seconds);
        }

        SDL_SetRenderDrawColor(renderer, 0,0,0,0);
        SDL_RenderClear(renderer);
        draw_lights(renderer);
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}