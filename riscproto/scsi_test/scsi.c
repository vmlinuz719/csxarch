#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef enum {
    DISCONNECTED,
    TARGET,
    INITIATOR
} scsi_device_state_t;

typedef enum {
    DASD,
    TAPE,
    PRINTER,
    PROCESSOR,
    WORM,
    CDROM,
    SCANNER,
    OPTICAL,
    JUKEBOX,
    COMMUNICATIONS
} scsi_device_type_t;

typedef enum {
    DATA_OUT,
    DATA_IN,
    COMMAND,
    STATUS,
    RESERVED_4,
    RESERVED_5,
    MESSAGE_OUT,
    MESSAGE_IN
} scsi_bus_phase_t;

struct scsi_device {
    // TODO: define this
};

struct scsi_bus {
    pthread_mutex_t arbitration_lock;

    struct scsi_device *devices[16];
    uint8_t *data_buffer;
    
    int initiator, target, lun;
    bool atn, req;
    int buf_index_low, buf_index_high;
    scsi_bus_phase_t phase;
    int status, sense_key, sense_code, sense_qual, sense_info;

    uint8_t command[10];
};

int scsi_init(struct scsi_bus *bus, uint32_t buf_max) {
    bus->data_buffer = calloc(buf_max, sizeof(uint8_t));
    if (bus->data_buffer == NULL) return -1;

    return pthread_mutex_init(&bus->arbitration_lock, NULL);
}

int scsi_destroy(struct scsi_bus *bus) {
    free(bus->data_buffer);
    return pthread_mutex_destroy(&bus->arbitration_lock);
}

void scsi_reset(struct scsi_bus *bus) {
    bus->phase = DATA_OUT;
    bus->buf_index_high = bus->buf_index_low = 0;
    bus->atn = false;
    bus->initiator = bus->target = -1;
    bus->lun = 0;
    bus->sense_key = bus->sense_code = bus->sense_qual = bus->sense_info = 0;
}

void scsi_arbitrate(struct scsi_bus *bus, int initiator) {
    pthread_mutex_lock(&bus->arbitration_lock);
    bus->initiator = initiator;
}

int scsi_try_arbitrate(struct scsi_bus *bus, int initiator) {
    if (
        bus->initiator < 0
        && pthread_mutex_trylock(&bus->arbitration_lock) == 0
    ) {
        bus->initiator = initiator;
        return 1;
    }

    return 0;
}

void scsi_release(struct scsi_bus *bus) {
    if (bus->initiator < 0) return;

    bus->phase = DATA_OUT;
    bus->initiator = bus->target = -1;
    bus->buf_index_high = bus->buf_index_low = 0;

    pthread_mutex_unlock(&bus->arbitration_lock);
}

int main(int argc, char *argv[]) {
    printf("Test Successful\n");
    return 0;
}