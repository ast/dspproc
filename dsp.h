//
//  dsp.h
//  fcdsdrosx
//
//  Created by Albin Stigö on 24/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef dsp_h
#define dsp_h

#include <stdio.h>
#include <complex.h>
#include "alsa.h"

typedef struct {
    float v;
    float u;
    float k1;
    float k2;
} osc_t;

typedef struct {
    
} dc_block_t;

typedef struct {
    float ref;
    float rate;
    float gain;
} agc2_t;

void dsp_agc2_init(agc2_t *a);

static inline float complex dsp_agc2(agc2_t *a, float complex in) {
    float complex res;
    float pow;
    res = a->gain * in;
    pow = crealf(res * conjf(res));
    a->gain += (a->ref - pow) * a->rate;
    return res;
}

void dsp_osc_init(osc_t *o, float freq, float rate);
float complex dsp_osc_next(osc_t *o);

float dsp_hann(int n, int N);
float dsp_power(float *h, int N);

void dsp_i24_to_c(float complex *dst, float complex *dst_win, frame_t *src, float *win, int len);

void dsp_i24_to_c_neon(float complex *dst, float complex *dst_win, frame_t *src, float complex *win, int len);

void dsp_cabs_neon(float *dst, float complex *src, int len);
void dsp_mag_squared_neon(float *dst, float complex* src, int len);

#endif /* dsp_h */
