#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define RX_BUF_SIZE 2048

typedef struct port {
    void *ctx;
    void (*port_write) (void *, uint8_t);
} slim_port_t;

void port_write(slim_port_t *port, uint8_t data) {
    port->port_write(port->ctx, data);
}

/*
    Write to router port:
        Accept byte into rx buffer
        If second byte:
            Select tx port, set size
            Mark source port in header
        If size reached:
            Secure tx port lock
            Write rx buffer to tx port
            Release tx lock
*/

typedef struct tx_ports {
    slim_port_t p[16];
    pthread_mutex_t tx_lock[16];
} tx_ports_t;

typedef struct rx_port_ctx {
    tx_ports_t *tx;
    uint8_t buf[RX_BUF_SIZE];
    uint16_t index;
    int id, size, tgt;
} rx_port_ctx_t;

typedef struct router {
    tx_ports_t tx;
    rx_port_ctx_t rx_ctx[16];
    slim_port_t rx[16];
} slim_router_t;

void router_rx_write(void *vctx, uint8_t ch) {
    rx_port_ctx_t *ctx = (rx_port_ctx_t *) vctx;

    ctx->buf[ctx->index++] = ch;
    if (!(ctx->size) && ctx->index == 2) {
        uint16_t header = ((uint16_t) (ctx->buf[0])) << 8;
        header |= (uint16_t) (ctx->buf[1]);

        uint16_t size = header & 0x8FF;
        if (size == 0) {
            ctx->size = 8;
        } else if (size == 0x8FF) {
            ctx->index = 0;
            ctx->size = 0;
        } else {
            ctx->size = size;
        }
        ctx->tgt = header >> 12;
    } else if (ctx->size && ctx->index >= ctx->size) {
        pthread_mutex_lock(&(ctx->tx->tx_lock[ctx->tgt]));

        slim_port_t *tx = &(ctx->tx->p[ctx->tgt]);

        uint8_t first_byte = ctx->buf[0] & 0xF;
        first_byte |= ctx->id << 4;
        port_write(tx, first_byte);

        for (int i = 1; i < ctx->size; i++) {
            port_write(tx, ctx->buf[i]);
        }

        pthread_mutex_unlock(&(ctx->tx->tx_lock[ctx->tgt]));

        ctx->index = 0;
        ctx->size = 0;
    }
}
