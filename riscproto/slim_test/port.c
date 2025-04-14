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

