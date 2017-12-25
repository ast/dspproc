//
//  circularbuffer.c
//  libdtmf
//
//  Created by Albin Stigö on 14/10/15.
//  Copyright © 2015 Albin Stigo. All rights reserved.
//

#include "circular_buffer.h"

#include <unistd.h>
#include <string.h>

// Create
bool cb_init(circular_buffer_t *cb, uint32_t size) {
    // Virtual memory magic starts here.
    char  path[] = "/tmp/cb-XXXXXX";
    int   fd;
    void *buf;
    void *adr;
    
    // Create temp file
    if((fd = mkstemp(path)) < 0) return false;
    // Remove link from filesystem
    if(unlink(path) < 0) return false;
    // Set size
    if(ftruncate(fd, size) < 0) return false;
    
    // Try to map an area of memory of 2 * size.
    buf = mmap(NULL, size << 1, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(buf == MAP_FAILED) return false;
    // zero memroy
    
    // Then map size bytes twice.
    adr = mmap(buf, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (adr != buf) return false;

    adr = mmap(buf + size , size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (adr != buf + size) return false;

    if(close(fd)) return false;

    memset(buf, 0, size);
    
    cb->read = 0;
    cb->write = 0;
    cb->size = size;
    cb->buffer = buf;

    return true;
}

void cb_destroy(circular_buffer_t *cb) {
    munlock((const void *) cb->buffer, cb->size << 1);
    // clear struct
    memset(cb, 0, sizeof(circular_buffer_t));
}
