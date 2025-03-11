#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
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

int main(int argc, char *argv[]) {
    printf("Test Successful\n");
    return 0;
}