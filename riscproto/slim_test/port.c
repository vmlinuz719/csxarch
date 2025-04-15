#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
    port p[16];
    pthread_mutex_t tx_lock[16];
} tx_ports_t;

typedef struct rx_port_ctx {
    tx_ports_t *tx;
    uint8_t buf[2048];
    uint16_t index;
}


