#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define RX_BUF_SIZE 2048
#define ROUTER_PORTS 16

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
    slim_port_t p[ROUTER_PORTS];
    pthread_mutex_t tx_lock[ROUTER_PORTS];
} tx_ports_t;

typedef struct rx_port_ctx {
    tx_ports_t *tx;
    uint8_t buf[RX_BUF_SIZE];
    uint16_t index;
    int id, size, tgt;
} rx_port_ctx_t;

typedef struct router {
    tx_ports_t tx;
    rx_port_ctx_t rx_ctx[ROUTER_PORTS];
    slim_port_t rx[ROUTER_PORTS];
} slim_router_t;

void null_write(void *vctx, uint8_t ch) {
    return;
}

void router_rx_write(void *vctx, uint8_t ch) {
    rx_port_ctx_t *ctx = (rx_port_ctx_t *) vctx;

    ctx->buf[ctx->index++] = ch;
    if (!(ctx->size) && ctx->index == 2) {
        uint16_t header = ((uint16_t) (ctx->buf[0])) << 8;
        header |= (uint16_t) (ctx->buf[1]);

        uint16_t size = header & 0x7FF;
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

typedef struct hexdump_port_ctx {
    pthread_mutex_t *hexdump_lock;
    uint8_t buf[RX_BUF_SIZE];
    uint16_t index;
    int id, size;
} hexdump_ctx_t;

void hexdump_write(void *vctx, uint8_t ch) {
    hexdump_ctx_t *ctx = (hexdump_ctx_t *) vctx;

    ctx->buf[ctx->index++] = ch;
    if (!(ctx->size) && ctx->index == 2) {
        uint16_t header = ((uint16_t) (ctx->buf[0])) << 8;
        header |= (uint16_t) (ctx->buf[1]);

        uint16_t size = header & 0x7FF;
        if (size == 0) {
            ctx->size = 8;
        } else if (size == 0x8FF) {
            ctx->index = 0;
            ctx->size = 0;
        } else {
            ctx->size = size;
        }
    } else if (ctx->size && ctx->index >= ctx->size) {
        pthread_mutex_lock(ctx->hexdump_lock);

        printf("%d:\n", ctx->id);

        for (int i = 0; i < ctx->size; i++) {
            printf("%02hhX ", ctx->buf[i]);
            if (i % 16 == 15 || i + 1 == ctx->size) {
                printf("\n");
            }
        }

        pthread_mutex_unlock(ctx->hexdump_lock);

        ctx->index = 0;
        ctx->size = 0;
    }
}

void hexdump_port_init(hexdump_ctx_t *ctx) {
    pthread_mutex_init(ctx->hexdump_lock, NULL);
}

void hexdump_port_destroy(hexdump_ctx_t *ctx) {
    pthread_mutex_destroy(ctx->hexdump_lock);
}

void router_init(slim_router_t *router) {
    for (int i = 0; i < ROUTER_PORTS; i++) {
        router->rx[i].ctx = &(router->rx_ctx[i]);
        router->rx[i].port_write = router_rx_write;

        router->rx_ctx[i].tx = &(router->tx);
        router->rx_ctx[i].id = i;
        router->rx_ctx[i].index = 0;
        router->rx_ctx[i].size = 0;

        router->tx.p[i].ctx = NULL;
        router->tx.p[i].port_write = null_write;
        pthread_mutex_init(&(router->tx.tx_lock[i]), NULL);
    }
}

void router_destroy(slim_router_t *router) {
    for (int i = 0; i < ROUTER_PORTS; i++) {
        pthread_mutex_destroy(&(router->tx.tx_lock[i]));
    }
}

int main(int argc, char *argv[]) {
    slim_router_t router;
    router_init(&router);
    
    pthread_mutex_t hexdump_mutex;
    hexdump_ctx_t hex_port;
    hex_port.hexdump_lock = &hexdump_mutex;
    hexdump_port_init(&hex_port);

    router.tx.p[2].ctx = &hex_port;
    router.tx.p[2].port_write = hexdump_write;

    port_write(&(router.rx[5]), 0x28);
    port_write(&(router.rx[5]), 0x00);
    port_write(&(router.rx[5]), 0x00);
    port_write(&(router.rx[5]), 0x00);
    port_write(&(router.rx[5]), 0xDE);
    port_write(&(router.rx[5]), 0xAD);
    port_write(&(router.rx[5]), 0xBE);
    port_write(&(router.rx[5]), 0xEF);

    hexdump_port_destroy(&hex_port);
    router_destroy(&router);
    return 0;
}