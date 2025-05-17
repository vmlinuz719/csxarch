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
#define SCREEN_HEIGHT 128

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
    
#define PANEL_ROWS 64
#define PANEL_VPAD 5
#define PANEL_HPAD 1
#define LED_HEIGHT 8
#define LED_WIDTH  8
    
    uint64_t panel_rows[PANEL_ROWS];
    int start_led[PANEL_ROWS] = {
        0, 32, 48, 0, 0, 0, 0, 59,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    int end_led[PANEL_ROWS] = {
        62, 64, 64, 0, 64, 64, 64, 64,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    int row_color[PANEL_ROWS][3] = {
        {255, 149, 66},
        {255, 149, 66},
        {255, 149, 66},
        {0, 0, 0},
        {66, 149, 255},
        {66, 149, 255},
        {66, 149, 255},
        {66, 255, 149},
    };
    
    int selection = 1;
    while (blink_ctx->running) {
        SDL_RenderClear(blink_ctx->render);
        
        panel_rows[0] = blink_ctx->cpu->pc;
        panel_rows[1] = blink_ctx->cpu->inst;
        panel_rows[2] = blink_ctx->cpu->c_regs[CR_PSQ];
        panel_rows[4] = blink_ctx->cpu->regs[R_ABI_SP];
        panel_rows[5] = blink_ctx->cpu->regs[R_ABI_LR];
        panel_rows[6] = blink_ctx->cpu->regs[selection];
        panel_rows[7] = selection;
        
        for (int j = 0; j < PANEL_ROWS; j++) {
            for (int i = start_led[j]; i < end_led[j]; i++) {
                SDL_Rect rect;
                rect.x = PANEL_VPAD + (PANEL_HPAD + LED_WIDTH) * i;
                rect.y = PANEL_VPAD + (LED_HEIGHT + PANEL_VPAD) * j;
                rect.w = LED_WIDTH;
                rect.h = LED_HEIGHT;
                if (panel_rows[j] & (0x8000000000000000 >> i)) {
                    SDL_SetRenderDrawColor
                            (blink_ctx->render,
                            row_color[j][0],
                            row_color[j][1],
                            row_color[j][2],
                            255);
                    SDL_RenderFillRect(blink_ctx->render, &rect);
                } else {
                    SDL_SetRenderDrawColor(blink_ctx->render, 64, 64, 64, 255);
                    SDL_RenderDrawRect(blink_ctx->render, &rect);
                }
                
            }
        }
        
        SDL_SetRenderDrawColor(blink_ctx->render, 0, 0, 0, 255);

        SDL_RenderPresent(blink_ctx->render);
        
        SDL_Event event;
        while( SDL_PollEvent( &event ) ){
            switch( event.type ){
                /* Look for a keypress */
                case SDL_KEYDOWN:
                    /* Check the SDLKey values and move change the coords */
                    switch( event.key.keysym.sym ){
                        /*
                        case SDLK_LEFT:
                            alien_x -= 1;
                            break;
                        case SDLK_RIGHT:
                            alien_x += 1;
                            break;
                        */
                        case SDLK_UP:
                            if (selection < R_ABI_SP - 1) selection++;
                            break;
                        case SDLK_DOWN:
                            if (selection > 1) selection--;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    blink_ctx->running = 0;
                    break;
            }
        }
    }

    SDL_DestroyRenderer(blink_ctx->render);    
    SDL_DestroyWindow(blink_ctx->window);
    SDL_Quit();
    return NULL;
}

void destroy_blink(void *ctx) {
    blink_ctx_t *blink_ctx = (blink_ctx_t *) ctx;
    SDL_Delay(1000);
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
