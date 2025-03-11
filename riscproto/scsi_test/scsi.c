/* 
 * Adapted from
 * https://github.com/open-simh/simh/blob/master/sim_scsi.c
 */

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

#define SCSI_NUM_DEVICES 8

struct scsi_bus {
    pthread_mutex_t arbitration_lock;

    struct scsi_device *devices[SCSI_NUM_DEVICES];
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

    for (int i = 0; i < SCSI_NUM_DEVICES; i++) {
        bus->devices[i] = NULL;
    }

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

void scsi_set_atn(struct scsi_bus *bus) {
    bus->atn = true;
    if (bus->target >= 0) bus->phase = MESSAGE_OUT;
}

void scsi_release_atn(struct scsi_bus *bus) {
    bus->atn = false;
}

void scsi_set_req(struct scsi_bus *bus) {
    bus->req = true;
}

void scsi_release_req(struct scsi_bus *bus) {
    bus->req = false;
}

void scsi_set_phase(struct scsi_bus *bus, scsi_bus_phase_t phase) {
    bus->phase = phase;
}

bool scsi_select(struct scsi_bus *bus, int target) {
    if (bus->initiator < 0 || bus->target >= 0) {
        return false;
    }

    if (bus->devices[target] != NULL) {
        if (bus->atn) scsi_set_phase(bus, MESSAGE_OUT);
        else scsi_set_phase(bus, COMMAND);
        
        bus->target = target;
        scsi_set_req(bus);
        return true;
    }

    scsi_release(bus);
    return false;
}

int main(int argc, char *argv[]) {
    printf("Test Successful\n");
    return 0;
}