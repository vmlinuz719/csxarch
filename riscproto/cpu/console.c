#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>

#include "lcca.h"
#include "mmio.h"
#include "error.h"
#include "console.h"

#define ECHO_ALL_BUT_TAB_AND_RETURN(x) (((x) >> 7) & 1)
#define ECHO_TAB(x) (((x) >> 6) & 1)
#define ECHO_RETURN(x) (((x) >> 5) & 1)
#define DESTRUCTIVE(x) (((x) >> 4) & 1)
#define LOCKED(x) (((x) >> 3) & 1)

#define INTR_RET(x) (((x) >> 2) & 1)
#define INTR_ESC(x) (((x) >> 1) & 1)
#define INTR_ANY(x) ((x) & 1)

typedef struct {
    int write_ready;
    struct termios oldt;
    uint8_t ctrl_byte;
    
    uint8_t buffer[BUF_SIZE];
    int current_in, current_out, current_size;
    
    uint8_t prt_buf[PRT_BUF_SIZE];
    int prt_in, prt_out, prt_size;
    
    pthread_t read_thread, prt_thread;
    pthread_mutex_t buffer_mutex, prt_buf_mutex;
    pthread_cond_t prt_ready;
    int running;

    lcca_t *cpu;
    uint64_t vec;
} console_ctx_t;

uint64_t console_read
        (void *ctx, int size, uint32_t addr, lcca_error_t *e) {
    // printf("console read %X %d\n", addr, size);
    console_ctx_t *console_ctx = (console_ctx_t *) ctx;
    if (addr == 0 && size == 1) {

        uint64_t result = 0;
        
        if (console_ctx->current_size) result |= CON_READ_READY;
        
        if (console_ctx->prt_size < PRT_BUF_SIZE) result |= CON_WRITE_READY;

        if (console_ctx->current_size == BUF_SIZE) result |= CON_INPUT_FULL;
        
        return result;
    }
    else if (addr == 1 && size == 1) {
        uint64_t result = 0;
        
        pthread_mutex_lock(&(console_ctx->buffer_mutex));
        if (console_ctx->current_size) {
            console_ctx->current_size--;
            if (console_ctx->current_size == BUF_SIZE - 1) {
                putchar(0x11);
            }
            result = console_ctx->buffer[console_ctx->current_out];
            console_ctx->current_out =
                (console_ctx->current_out + 1) % BUF_SIZE;
        }
        pthread_mutex_unlock(&(console_ctx->buffer_mutex));
        
        return result;
    }
    else if (addr == 2 && size == 1) {
        return console_ctx->ctrl_byte;
    }
    else if (addr == 8 && size == 8) {
        return console_ctx->vec;
    }
    else {
        *e = BERR;
        return 0;
    }
}

void console_write
        (void *ctx, int size, uint32_t addr, uint64_t data,
            lcca_error_t *e) {
    console_ctx_t *console_ctx = (console_ctx_t *) ctx;
    if (addr == 2 && size == 1) {
        console_ctx->ctrl_byte = data & 0xFF;
    }
    else if (addr == 3 && size == 1 && console_ctx->write_ready == 1) {
        /*
        console_ctx->write_ready = 0;
        char c = data & 0xFF;
        int x = write(1, &c, 1);
        (void) x;
        console_ctx->write_ready = 1;
        */
        pthread_mutex_lock(&(console_ctx->prt_buf_mutex));
        
        if (console_ctx->prt_size < PRT_BUF_SIZE) {
            if (console_ctx->prt_size++ == 0) {
                pthread_cond_signal(&(console_ctx->prt_ready));
            }
            
            console_ctx->prt_buf[console_ctx->prt_in] = (uint8_t) data;
            console_ctx->prt_in = (console_ctx->prt_in + 1) % PRT_BUF_SIZE;
        }
        
        // TODO: else: interrupt
        
        pthread_mutex_unlock(&(console_ctx->prt_buf_mutex));
    }
    else if (addr == 8 && size == 8) {
        console_ctx->vec = data;
    }
    else {
        // should interrupt here if enabled
        *e = BERR;
    }
}

void echo(int c, console_ctx_t *console_ctx) {
    if (c != '\n' && c != '\t' && c != 0x1B
        && ECHO_ALL_BUT_TAB_AND_RETURN(console_ctx->ctrl_byte)) {
        if ((c == 0x7F || c == '\b') && DESTRUCTIVE(console_ctx->ctrl_byte)) {
            if (!LOCKED(console_ctx->ctrl_byte)) {
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else if (c == 0x7F || c == '\b') {
            if (!LOCKED(console_ctx->ctrl_byte)) putchar('\b');
        }
        else putchar(c);
    }
    else if (c == '\n' && ECHO_RETURN(console_ctx->ctrl_byte)) putchar(c);
    else if (c == '\t' && ECHO_TAB(console_ctx->ctrl_byte)) putchar(c);
}

void *console_thread(void *ctx) {
    console_ctx_t *console_ctx = (console_ctx_t *) ctx;

    while (console_ctx->running) {
        int c = getchar();
        if (c == 0x7F) c = '\b';

        pthread_mutex_lock(&(console_ctx->buffer_mutex));
        if (console_ctx->current_size == BUF_SIZE) {
            putchar(0x07);
        }
        else {
            console_ctx->buffer[console_ctx->current_in] = c;
            console_ctx->current_in = (console_ctx->current_in + 1) % BUF_SIZE;
            console_ctx->current_size++;
            
            if (console_ctx->current_size == BUF_SIZE) {
                putchar(0x13);
            }
            
            if ((c == '\n' && INTR_RET(console_ctx->ctrl_byte))
                || (c == 0x1B && INTR_ESC(console_ctx->ctrl_byte))
                || (INTR_ANY(console_ctx->ctrl_byte))
                || (console_ctx->current_size == BUF_SIZE)
            ) {
                lcca_intr(console_ctx->cpu, CONSOLE_IRQ, console_ctx->vec);
            }

            if (c != 0x1B) echo(c, console_ctx);
        }
        pthread_mutex_unlock(&(console_ctx->buffer_mutex));
        if (c == EOF) break;
    }
    
    return NULL;
}

void *prt_thread(void *ctx) {
    console_ctx_t *console_ctx = (console_ctx_t *) ctx;

    while (console_ctx->running) {
        pthread_mutex_lock(&(console_ctx->prt_buf_mutex));
        if (!(console_ctx->prt_size)) pthread_cond_wait(
            &(console_ctx->prt_ready),
            &(console_ctx->prt_buf_mutex)
        );
        
        while (console_ctx->prt_size) {
            --console_ctx->prt_size;
            char c = console_ctx->prt_buf[console_ctx->prt_out];
            console_ctx->prt_out = (console_ctx->prt_out + 1) % PRT_BUF_SIZE;
            
            pthread_mutex_unlock(&(console_ctx->prt_buf_mutex));
            int x = write(1, &c, 1);
            (void) x;
            pthread_mutex_lock(&(console_ctx->prt_buf_mutex));
        }
        
        pthread_mutex_unlock(&(console_ctx->prt_buf_mutex));
    }
    
    return NULL;
}

uint64_t console_sense(void *ctx, int reg) {
    if (reg == 0) {
        return 0x1896;
    } else {
        return 0;
    }
}

void destroy_console(void *ctx) {
    console_ctx_t *console_ctx = (console_ctx_t *) ctx;
    console_ctx->running = 0;
    pthread_cancel(console_ctx->read_thread);
    pthread_join(console_ctx->read_thread, NULL);
    pthread_cancel(console_ctx->prt_thread);
    pthread_join(console_ctx->prt_thread, NULL);
    pthread_mutex_destroy(&(console_ctx->buffer_mutex));
    pthread_mutex_destroy(&(console_ctx->prt_buf_mutex));
    pthread_cond_destroy(&(console_ctx->prt_ready));
    tcsetattr(0, TCSANOW, &(console_ctx->oldt));
    free(console_ctx);
}

void init_console(mmio_unit_t *u, lcca_t *cpu) {
    console_ctx_t *ctx = malloc(sizeof(console_ctx_t));
    ctx->write_ready = 1;
    u->ctx = ctx;
    u->read = console_read;
    u->write = console_write;
    u->command = NULL;
    u->destroy = destroy_console;

    pthread_mutex_init(&(ctx->buffer_mutex), NULL);
    ctx->running = 1;
    ctx->current_in = 0;
    ctx->current_out = 0;
    ctx->current_size = 0;
    ctx->ctrl_byte = 0xF0;
    
    ctx->cpu = cpu;
    ctx->vec = 0;

    pthread_mutex_init(&(ctx->prt_buf_mutex), NULL);
    pthread_cond_init(&(ctx->prt_ready), NULL);
    ctx->prt_in = 0;
    ctx->prt_out = 0;
    ctx->prt_size = 0;
    
    tcgetattr(0, &(ctx->oldt));
    struct termios newt = ctx->oldt;
    newt.c_lflag &= ~(ICANON) & ~(ECHO);
    tcsetattr(0, TCSANOW, &newt);

    pthread_create(&(ctx->read_thread), NULL, console_thread, ctx);
    pthread_create(&(ctx->prt_thread), NULL, prt_thread, ctx);
}
