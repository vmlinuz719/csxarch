#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "csx.h"
#include "mmio.h"
#include "error.h"
#include "disk.h"

typedef struct {
    // TODO: Stone soup
    disk_lun_t lun[DISK_NUM_LUN];
    uint64_t cmd_address;
    uint64_t lrc, erd; // Last Return Code, Extra Response Data
    int run, ready, attn;
    int irq, vec;
    pthread_t cmd_thread;
    pthread_mutex_t cmd_mutex;
    em3_regs_t *cpu;
} disk_ctx_t;

typedef struct {
    uint64_t buf_address;
    uint8_t opcode, lun;
    int advance, chain;
} disk_cmd_hdr_t;

uint64_t disk_read
        (void *ctx, int size, uint32_t addr, em3_access_error_t *e) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;
    
    if (addr == 8 && size == 8) {
        return ((disk_ctx->irq & 0xF) << 8) | (disk_ctx->vec & 0x3F);
    }

    else {
        *e = BUS_ERROR;
        return 0;
    }
}

void read_cmd(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, em3_access_error_t *e) {
    c->buf_address = read_8b(disk_ctx->cpu, disk_ctx->cmd_address, e);
    if (*e) { return; }
    
    uint32_t cmd_header = read_u4b(disk_ctx->cpu, disk_ctx->cmd_address + 8, e);
    if (*e) { return; }
    
    c->opcode = (cmd_header >> 24) & 0xFF;
    c->advance = (cmd_header >> 23) & 1;
    c->chain = (cmd_header >> 22) & 1;
    c->lun = (cmd_header >> 16) & 0x3F;
    
    return;
}

uint64_t do_cmd_media(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, uint64_t *next) {
    em3_access_error_t e = OK;
    
    if (disk_ctx->lun[c->lun].image != NULL) {
        uint64_t result = (
            (disk_ctx->lun[c->lun].num_sectors - 1)
            | (((uint64_t) disk_ctx->lun[c->lun].sector_size) << 48)
            | (((uint64_t)(disk_ctx->lun[c->lun].write_protect & 1)) << 48)
        );
        
        write_8b(
            disk_ctx->cpu,
            c->buf_address, 
            result,
            &e
        );
        
        if (e) {
            *next = c->buf_address;
            return DISK_RC_BUS_ERROR;
        }
    }
    
    c->buf_address += 8;
    *next = disk_ctx->cmd_address + 16;
    return DISK_RC_SUCCESS;
}

uint64_t do_cmd_read_ipl(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, uint64_t *next) {
    em3_access_error_t e = OK;
    
    uint32_t count = read_u4b(disk_ctx->cpu, disk_ctx->cmd_address + 12, &e);
    if (e) {
        *next = disk_ctx->cmd_address + 12;
        return DISK_RC_BUS_ERROR;
    }
    
    if (disk_ctx->lun[c->lun].image == NULL) {
        *next = c->lun;
        return DISK_RC_NOT_READY;
    }
    
    FILE *fp = disk_ctx->lun[c->lun].image;
    fseek(fp, 0, SEEK_SET);
    
    uint8_t *blk_buf = malloc(disk_ctx->lun[c->lun].sector_size);
    
    uint64_t result = DISK_RC_SUCCESS;
    for (uint32_t index = 0; index < count; index++) {
        int read = fread(blk_buf, 1, disk_ctx->lun[c->lun].sector_size, fp);
        
        if (feof(fp) || read < disk_ctx->lun[c->lun].sector_size) {
            result = DISK_RC_SECTOR_OVERRUN;
            *next = index;
            break;
        } else if (ferror(fp)) {
            result = DISK_RC_MEDIUM_CHECK;
            *next = index;
            break;
        }
        
        for (int i = 0; i < disk_ctx->lun[c->lun].sector_size; i++) {
            write_1b(disk_ctx->cpu, c->buf_address, blk_buf[i], &e);
            if (e) {
                *next = c->buf_address;
                result = DISK_RC_BUS_ERROR;
                break;
            } else {
                c->buf_address++;
            }
        }
        
        if (e) {
            break;
        }
    }
    
    free(blk_buf);
    
    if (result == DISK_RC_SUCCESS) {
        *next = disk_ctx->cmd_address + 16;
    }
    
    return result;
}

uint64_t do_cmd_read(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, uint64_t *next) {
    em3_access_error_t e = OK;
    
    uint32_t count = read_u4b(disk_ctx->cpu, disk_ctx->cmd_address + 12, &e);
    if (e) {
        *next = disk_ctx->cmd_address + 12;
        return DISK_RC_BUS_ERROR;
    }
    
    uint64_t seek = read_8b(disk_ctx->cpu, disk_ctx->cmd_address + 16, &e) & 0xFFFFFFFFFFFF;
    if (e) {
        *next = disk_ctx->cmd_address + 16;
        return DISK_RC_BUS_ERROR;
    }
    
    if (disk_ctx->lun[c->lun].image == NULL) {
        *next = c->lun;
        return DISK_RC_NOT_READY;
    }
    
    if (seek >= disk_ctx->lun[c->lun].num_sectors) {
        *next = seek;
        return DISK_RC_SECTOR_OVERRUN;
    }
    
    FILE *fp = disk_ctx->lun[c->lun].image;
    fseek(fp, seek * disk_ctx->lun[c->lun].sector_size, SEEK_SET);
    
    uint8_t *blk_buf = malloc(disk_ctx->lun[c->lun].sector_size);
    
    uint64_t result = DISK_RC_SUCCESS;
    
    int read = fread(blk_buf, 1, disk_ctx->lun[c->lun].sector_size, fp);
    
    if (feof(fp) || read < disk_ctx->lun[c->lun].sector_size) {
        result = DISK_RC_SECTOR_OVERRUN;
        *next = seek;
    } else if (ferror(fp)) {
        result = DISK_RC_MEDIUM_CHECK;
        *next = seek;
    }
    
    if (result == DISK_RC_SUCCESS) {
        for (int i = 0; i < disk_ctx->lun[c->lun].sector_size; i++) {
            write_1b(disk_ctx->cpu, c->buf_address, blk_buf[i], &e);
            if (e) {
                *next = c->buf_address;
                result = DISK_RC_BUS_ERROR;
                break;
            } else {
                c->buf_address++;
            }
        }
    }
    
    free(blk_buf);
    
    if (result == DISK_RC_SUCCESS) {
        if (count) {
            write_4b(disk_ctx->cpu, disk_ctx->cmd_address + 12, count - 1, &e);
            if (e) {
                *next = disk_ctx->cmd_address + 12;
                return DISK_RC_BUS_ERROR;
            }
            
            seek = (seek + 1) & 0xFFFFFFFFFFFF;
            write_8b(disk_ctx->cpu, disk_ctx->cmd_address + 16, seek, &e);
            if (e) {
                *next = disk_ctx->cmd_address + 16;
                return DISK_RC_BUS_ERROR;
            }
            
            *next = disk_ctx->cmd_address;
        } else {
            *next = disk_ctx->cmd_address + 24;
        }
    }
    
    return result;
}

uint64_t do_cmd_write(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, uint64_t *next) {
    em3_access_error_t e = OK;
    
    uint32_t count = read_u4b(disk_ctx->cpu, disk_ctx->cmd_address + 12, &e);
    if (e) {
        *next = disk_ctx->cmd_address + 12;
        return DISK_RC_BUS_ERROR;
    }
    
    uint64_t seek = read_8b(disk_ctx->cpu, disk_ctx->cmd_address + 16, &e) & 0xFFFFFFFFFFFF;
    if (e) {
        *next = disk_ctx->cmd_address + 16;
        return DISK_RC_BUS_ERROR;
    }
    
    if (disk_ctx->lun[c->lun].image == NULL) {
        *next = c->lun;
        return DISK_RC_NOT_READY;
    }
    
    if (disk_ctx->lun[c->lun].write_protect) {
        *next = c->lun;
        return DISK_RC_WRITE_PROTECT;
    }
    
    if (seek >= disk_ctx->lun[c->lun].num_sectors) {
        *next = seek;
        return DISK_RC_SECTOR_OVERRUN;
    }
    
    FILE *fp = disk_ctx->lun[c->lun].image;
    fseek(fp, seek * disk_ctx->lun[c->lun].sector_size, SEEK_SET);
    
    uint8_t *blk_buf = malloc(disk_ctx->lun[c->lun].sector_size);
    
    uint64_t result = DISK_RC_SUCCESS;
    
    for (int i = 0; i < disk_ctx->lun[c->lun].sector_size; i++) {
        blk_buf[i] = read_u1b(disk_ctx->cpu, c->buf_address, &e);
        if (e) {
            *next = c->buf_address;
            free(blk_buf);
            return DISK_RC_BUS_ERROR;
        } else {
            c->buf_address++;
        }
    }
    
    int write = fwrite(blk_buf, 1, disk_ctx->lun[c->lun].sector_size, fp);
    fflush(fp);
    
    if (feof(fp) || write < disk_ctx->lun[c->lun].sector_size) {
        result = DISK_RC_SECTOR_OVERRUN;
        *next = seek;
    } else if (ferror(fp)) {
        result = DISK_RC_MEDIUM_CHECK;
        *next = seek;
    }
    
    free(blk_buf);
    
    if (result == DISK_RC_SUCCESS) {
        if (count) {
            write_4b(disk_ctx->cpu, disk_ctx->cmd_address + 12, count - 1, &e);
            if (e) {
                *next = disk_ctx->cmd_address + 12;
                return DISK_RC_BUS_ERROR;
            }
            
            seek = (seek + 1) & 0xFFFFFFFFFFFF;
            write_8b(disk_ctx->cpu, disk_ctx->cmd_address + 16, seek, &e);
            if (e) {
                *next = disk_ctx->cmd_address + 16;
                return DISK_RC_BUS_ERROR;
            }
            
            *next = disk_ctx->cmd_address;
        } else {
            *next = disk_ctx->cmd_address + 24;
        }
    }
    
    return result;
}

uint64_t do_cmd(disk_ctx_t *disk_ctx, disk_cmd_hdr_t *c, uint64_t *next) {
    switch (c->opcode) {
        case 0x1: {
            return do_cmd_media(disk_ctx, c, next);
        } break;
        
        case 0x4: {
            return do_cmd_read_ipl(disk_ctx, c, next);
        } break;
        
        case 0x5: {
            return do_cmd_read(disk_ctx, c, next);
        } break;
        
        case 0x8: {
            return do_cmd_write(disk_ctx, c, next);
        } break;
        
        default: {
            *next = c->opcode;
            return DISK_RC_UNSUPPORTED;
        }
    }
}

void *command_thread(void *ctx) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;
    int chaining = 1;
    
    do {
        em3_access_error_t e = OK;
        disk_cmd_hdr_t cmd;
        read_cmd(disk_ctx, &cmd, &e);
        
        if (e == OK) {
            // fprintf(stderr, "CSX4010: Got command 0x%01lX\n", disk_ctx->cmd_address);
            
            uint64_t next;
            disk_ctx->lrc = do_cmd(disk_ctx, &cmd, &next);
            
            write_2b(
                disk_ctx->cpu,
                disk_ctx->cmd_address + 10, 
                disk_ctx->lrc,
                &e
            );
            
            if (e) {
                disk_ctx->lrc = DISK_RC_BUS_ERROR;
                next = disk_ctx->cmd_address + 10;
            } else if (cmd.advance) {
                write_8b(
                    disk_ctx->cpu,
                    disk_ctx->cmd_address, 
                    cmd.buf_address,
                    &e
                );
                
                if (e) {
                    disk_ctx->lrc = DISK_RC_BUS_ERROR;
                    next = disk_ctx->cmd_address;
                }
            }
            
            if (
                disk_ctx->lrc != DISK_RC_SUCCESS
                && disk_ctx->lrc != DISK_RC_SUCCESS_EXTRA
            ) {
                disk_ctx->attn = 1;
                chaining = 0;
                disk_ctx->erd = next;
            } else {
                chaining = cmd.chain;
                if (chaining) {
                    disk_ctx->cmd_address = next;
                    
                    write_8b(
                        disk_ctx->cpu,
                        disk_ctx->cmd_address, 
                        cmd.buf_address,
                        &e
                    );
                    
                    if (e) {
                        disk_ctx->lrc = DISK_RC_BUS_ERROR;
                        disk_ctx->attn = 1;
                        chaining = 0;
                        disk_ctx->erd = disk_ctx->cmd_address;
                    }
                }
            }
        } else {
            // fprintf(stderr, "CSX4010: Command read error 0x%01X\n", e);
            disk_ctx->lrc = DISK_RC_BUS_ERROR;
            disk_ctx->attn = 1;
            chaining = 0;
        }
    } while (chaining);

    disk_ctx->ready = 1;
    
    pthread_mutex_unlock(&(disk_ctx->cmd_mutex));
    
    if (disk_ctx->vec) {
        intr(disk_ctx->cpu, disk_ctx->irq, disk_ctx->vec);
    }
    
    return NULL;
}

void disk_write
        (void *ctx, int size, uint32_t addr, uint64_t data,
            em3_access_error_t *e) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;

    if (addr == 8 && size == 8) {
        disk_ctx->irq = (data >> 8) & 0xF;
        disk_ctx->vec = data & 0x3F;
    } else if (addr == 0 && size == 8) {
        if (!pthread_mutex_trylock(&(disk_ctx->cmd_mutex))) {
            disk_ctx->cmd_address = data;
            disk_ctx->ready = 0;
            disk_ctx->attn = 0;
            pthread_create(
                &(disk_ctx->cmd_thread),
                NULL,
                command_thread,
                disk_ctx
            );
        } else {
            *e = BUS_ERROR;
        }
    } else {
        *e = BUS_ERROR;
    }
}

uint64_t disk_command
        (void *ctx, int reg, uint64_t data, em3_access_error_t *e) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;

    if (reg == 1) {
        disk_ctx->irq = (data >> 8) & 0xF;
        disk_ctx->vec = data & 0x3F;
    } else if (reg == 0) {
        if (!pthread_mutex_trylock(&(disk_ctx->cmd_mutex))) {
            disk_ctx->cmd_address = data;
            disk_ctx->ready = 0;
            disk_ctx->attn = 0;
            pthread_create(
                &(disk_ctx->cmd_thread),
                NULL,
                command_thread,
                disk_ctx
            );
        } else {
            *e = BUS_ERROR;
        }
    } else {
        *e = BUS_ERROR;
    }
}

uint64_t disk_sense(void *ctx, int reg) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;

    switch (reg) {
        case 0: return 0x4010;
        case 1: return (disk_ctx->cmd_address & 0xFFFFFFFFFFFFFFF8)
            | (disk_ctx->run << 2)
            | (disk_ctx->ready << 1)
            | (disk_ctx->attn);
        case 2: return disk_ctx->lrc;
        case 3: return disk_ctx->erd;
        case 15: return 0x4D414348464F5552;
        default: return 0;
    }
}

void destroy_disk(void *ctx) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) ctx;
    
    pthread_mutex_lock(&(disk_ctx->cmd_mutex));
    pthread_mutex_unlock(&(disk_ctx->cmd_mutex));
    
    for (int i = 0; i < DISK_NUM_LUN; i++) {
        if (disk_ctx->lun[i].image != NULL) {
            fclose(disk_ctx->lun[i].image);
        }
    }
    
    pthread_mutex_destroy(&(disk_ctx->cmd_mutex));
    free(disk_ctx);
}

void init_disk(mmio_unit_t *u, em3_regs_t *cpu) {
    disk_ctx_t *ctx = malloc(sizeof(disk_ctx_t));
    
    u->ctx = ctx;
    u->read = disk_read;
    u->write = disk_write;
    u->sense = disk_sense;
    u->command = disk_command;
    u->destroy = destroy_disk;
    
    ctx->cpu = cpu;
    ctx->irq = 0;
    ctx->vec = 0;
    ctx->run = 0;
    ctx->ready = 1;
    ctx->attn = 0;
    ctx->lrc = 0;
    ctx->erd = 0;
    
    for (int i = 0; i < DISK_NUM_LUN; i++) {
        ctx->lun[i].image = NULL;
    }
    
    pthread_mutex_init(&(ctx->cmd_mutex), NULL);
}

int disk_init_lun(mmio_unit_t *u, int lun, char *fname, size_t sector_size, int ro) {
    disk_ctx_t *disk_ctx = (disk_ctx_t *) u->ctx;
    
    disk_lun_t *plun = &(disk_ctx->lun[lun]);
    
    FILE *disk_image = fopen(fname, "rb+");
    if (disk_image == NULL) {
        return -1;
    }
    
    if (fseek(disk_image, 0, SEEK_END) < 0) {
        fclose(disk_image);
        return -1;
    }
    
    plun->num_sectors = ftell(disk_image) / sector_size;
    fseek(disk_image, 0, SEEK_SET);
    
    plun->image = disk_image;
    plun->sector_size = sector_size;
    plun->write_protect = ro;
}