//
//  fir_filter.h
//  cbtest
//
//  Created by Albin Stigö on 27/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef fir_filter_h
#define fir_filter_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <complex.h>
#include "circular_buffer.h"

typedef struct {
    circular_buffer_t cb;
    float *h;
    uint16_t n;
} fir_filter_t;

bool fir_init(fir_filter_t *f, float *h, uint16_t n);
void fir_destroy(fir_filter_t *f);
float complex* fir_push (fir_filter_t *f, uint32_t amount);
void fir_execute(fir_filter_t *f, float complex* dst, uint16_t len);


#endif /* fir_filter_h */
