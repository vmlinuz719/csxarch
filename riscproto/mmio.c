#include <stddef.h>
#include <stdint.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"

// TODO: figure out why the correct endianness isn't being detected in the header

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define __CSX_LITTLE_ENDIAN__
#endif

// TODO: Save bus/alignment error address

uint64_t read_u1b(lcca_bus_t *r, uint64_t a, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // no alignment required for bytes

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 1) {
            *e = BUS_ERROR;
            return 0;
        }

        uint64_t result = (uint64_t) m[a];
        return result;
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            return 0;
        }

        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            1,
            MMIO_REGISTER(a),
            e
        );

        if (*e) return 0;
        else return result;
    }
}

uint64_t read_u2b(lcca_bus_t *r, uint64_t a, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 2-byte alignment
    if (a % 2) {
        if (e != NULL) *e = ERR_ALIGN;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 2) {
            *e = BUS_ERROR;
            return 0;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        uint64_t result = (uint64_t) bswap_16(*((uint16_t *) (m + a)));
#else
        uint64_t result = (uint64_t) (*((uint16_t *) (m + a)));
#endif
        return result;
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            return 0;
        }

        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            2,
            MMIO_REGISTER(a),
            e
        );

        if (*e) return 0;
        else return result;
    }
}

uint64_t read_u4b(lcca_bus_t *r, uint64_t a, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 4-byte alignment
    if (a % 4) {
        if (e != NULL) *e = ERR_ALIGN;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 4) {
            *e = BUS_ERROR;
            return 0;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        uint64_t result = (uint64_t) bswap_32(*((uint32_t *) (m + a)));
#else
        uint64_t result = (uint64_t) (*((uint32_t *) (m + a)));
#endif
        return result;
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            return 0;
        }

        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            4,
            MMIO_REGISTER(a),
            e
        );

        if (*e) return 0;
        else return result;
    }
}

uint64_t read_8b(lcca_bus_t *r, uint64_t a, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 8-byte alignment
    if (a % 8) {
        if (e != NULL) *e = ERR_ALIGN;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 8) {
            *e = BUS_ERROR;
            return 0;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        uint64_t result = bswap_64(*((uint64_t *) (m + a)));
#else
        uint64_t result = (*((uint64_t *) (m + a)));
#endif
        return result;
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            return 0;
        }

        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            8,
            MMIO_REGISTER(a),
            e
        );

        if (*e) return 0;
        else return result;
    }
}

void write_1b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e) {
    uint8_t *m = r->memory;
    // no alignment required for bytes

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 1) {
            *e = BUS_ERROR;
            return;
        }

        m[a] = (uint8_t) v;
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            return;
        }

        r->mmio[unit].write(
            r->mmio[unit].ctx,
            1,
            MMIO_REGISTER(a),
            v,
            e
        );
    }
}

void write_2b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 2-byte alignment
    if (a % 2) {
        if (e != NULL) *e = ERR_ALIGN;
        return;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 2) {
            *e = BUS_ERROR;
            return;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        *((uint16_t *) (m + a)) = bswap_16((uint16_t) v);
#else
        *((uint16_t *) (m + a)) = ((uint16_t) v);
#endif
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            return;
        }

        r->mmio[unit].write(
            r->mmio[unit].ctx,
            2,
            MMIO_REGISTER(a),
            v,
            e
        );
    }
}

void write_4b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 4-byte alignment
    if (a % 4) {
        if (e != NULL) *e = ERR_ALIGN;
        return;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 4) {
            *e = BUS_ERROR;
            return;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        *((uint32_t *) (m + a)) = bswap_32((uint32_t) v);
#else
        *((uint32_t *) (m + a)) = ((uint32_t) v);
#endif
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            return;
        }

        r->mmio[unit].write(
            r->mmio[unit].ctx,
            4,
            MMIO_REGISTER(a),
            v,
            e
        );
    }
}

void write_8b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e) {
    uint8_t *m = r->memory;

    // require 8-byte alignment
    if (a % 8) {
        if (e != NULL) *e = ERR_ALIGN;
        return;
    }

    if (a < MMIO_BASE) {
        if (a > r->mem_limit - 8) {
            *e = BUS_ERROR;
            return;
        }

#ifdef __CSX_LITTLE_ENDIAN__
        *((uint64_t *) (m + a)) = bswap_64(v);
#else
        *((uint64_t *) (m + a)) = (v);
#endif
    }

    else {
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            return;
        }

        r->mmio[unit].write(
            r->mmio[unit].ctx,
            8,
            MMIO_REGISTER(a),
            v,
            e
        );
    }
}
