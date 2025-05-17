#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "lcca.h"
#include "mmio.h"
#include "error.h"
#include "blinkenlights.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct {
    int running;
    pthread_t thread;
    lcca_t *cpu;
    uint64_t vec;
    
    SDL_Window *window;
    SDL_Renderer *render;
} blink_ctx_t;

void *blink_thread(void *ctx) {
    blink_ctx_t *blink_ctx = (blink_ctx_t *) ctx;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return NULL;
    
    blink_ctx->window = SDL_CreateWindow(
        "craptangle",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (blink_ctx->window == NULL) {
        SDL_Quit();
        return NULL;
    }
    
    blink_ctx->render = SDL_CreateRenderer
            (blink_ctx->window, -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (blink_ctx->render == NULL) {
        SDL_DestroyRenderer(blink_ctx->render);
        SDL_Quit();
        return NULL;
    }
    
    int i = 0;
    while (blink_ctx->running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                blink_ctx->running = 0;
        }
        
        SDL_RenderClear(blink_ctx->render);
        
        SDL_Rect rect;
        rect.x = i;
        i = (i + 1) % 256;
        rect.y = 150;
        rect.w = 200;
        rect.h = 200;
        SDL_SetRenderDrawColor(blink_ctx->render, 255, 255, 255, 255);
        SDL_RenderDrawRect(blink_ctx->render, &rect);

        SDL_SetRenderDrawColor(blink_ctx->render, 0, 0, 0, 255);

        SDL_RenderPresent(blink_ctx->render);
    }

    SDL_DestroyRenderer(blink_ctx->render);    
    SDL_DestroyWindow(blink_ctx->window);
    SDL_Quit();
    return NULL;
}

void destroy_blink(void *ctx) {
    blink_ctx_t *blink_ctx = (blink_ctx_t *) ctx;
    blink_ctx->running = 0;
    pthread_join(blink_ctx->thread, NULL);
    free(blink_ctx);
}

void init_blink(mmio_unit_t *u, lcca_t *cpu) {
    blink_ctx_t *ctx = malloc(sizeof(blink_ctx_t));
    u->ctx = ctx;
    u->read = NULL;
    u->write = NULL;
    u->command = NULL;
    u->destroy = destroy_blink;

    ctx->running = 1;
    
    ctx->cpu = cpu;
    ctx->vec = 0;
    

    pthread_create(&(ctx->thread), NULL, blink_thread, ctx);
}
