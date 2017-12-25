//
//  circularbuffer.h
//  libdtmf
//
//  Created by Albin Stigö on 14/10/15.
//  Copyright © 2015 Albin Stigo. All rights reserved.
//

#ifndef circularbuffer_h
#define circularbuffer_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/*
 * Inspired by:
 * https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 * https://github.com/willemt/cbuffer/
 * https://github.com/michaeltyson/TPCircularBuffer
 */

typedef struct {
    void      *buffer;
    uint32_t  read;
    uint32_t  write;
    // size MUST be power of two!!
    uint32_t  size;
} circular_buffer_t;

// Creation
// size must be power of two!!
bool cb_init(circular_buffer_t *cb, uint32_t size);
void cb_destroy(circular_buffer_t *cb);

static inline uint32_t cb_mask(circular_buffer_t *cb, uint32_t val)
{
    return val & (cb->size - 1);
}

// Access
static inline void cb_produce(circular_buffer_t *cb, uint32_t amount)
{
    cb->write += amount;
}

static inline void cb_consume(circular_buffer_t *cb, uint32_t amount)
{
    cb->read += amount;
}

// Pointer to head ready for writing
static inline void* cb_writeptr(circular_buffer_t *cb)
{
    return &cb->buffer[cb_mask(cb, cb->write)];
}

// Pointer to tail, ready for reading
static inline void* cb_readptr(circular_buffer_t *cb)
{
    return &cb->buffer[cb_mask(cb, cb->read)];
}

#endif /* circularbuffer_h */
