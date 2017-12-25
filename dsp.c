//
//  dsp.c
//  fcdsdrosx
//
//  Created by Albin Stigö on 24/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include "dsp.h"
#include <math.h>
#include <arm_neon.h>

float z[2048] = {0};

void dsp_osc_init(osc_t *o, float freq, float rate) {
    float w = freq * (2 * M_PI / rate);
    o->u = 1;
    o->v = 0;
    o->k1 = tanf(0.5 * w);
    o->k2 = 2 * o->k1 / (1 + o->k1 * o->k1);
}

float complex dsp_osc_next(osc_t *o) {
    float tmp;
    tmp = o->u - o->k1 * o->v;
    o->v = o->v + o->k2 * tmp;
    o->u = tmp - o->k1 * o->v;
    return o->u + o->v * I;
}

float dsp_hann(int n, int N) {
    return 0.5 - 0.5 * cos (2 * M_PI * n/ ( N-1) );
}

float dsp_power(float *h, int N) {
    double acc = 0;
    for (int n = 0; n < N; n++) {
        acc += h[n]*h[n];
    }
    return acc;
}

// Reference implementation
void dsp_i24_to_c(float complex *dst, float complex *dst_win,
                       frame_t *src, float *win, int len) {
    static const float Q = 1.0 / 0x7fffff;
    for (int n = 0; n < len; n++) {
        dst[n] = (src[n].r >> 8) * Q + (src[n].l >> 8) * Q * I;
        dst_win[n] = dst[n] * win[n];
    }
}

void dsp_i24_to_c_neon(float complex *dst, float complex *dst_win,
                       frame_t *src, float complex *win, int len) {
    
    static const float Q = 1.0 / 0x7fffff;
    
    int32x4_t v, vshr;
    float32x4_t vf;
    for (int i = 0; i < len; i+=4) {
        // load vector
        v = vld1q_s32((int32_t*) &src[i]);
        // shift right by 8
        vshr = vshrq_n_s32(v, 8);
        // convert to float vector
        vf = vcvtq_f32_s32(vshr);
        // multiply by scaling constant
        vf = vmulq_n_f32(vf, Q);
        // write to first destination
        vst1q_f32((float*) &dst[i], vf);
        
        // load an apply window.
        // using float complex for pointer arithmetic
        float32x4_t w = vld1q_f32((float*) &win[i]);
        vf = vmulq_f32(vf, w);
        // store windowed
        vst1q_f32((float*) &dst_win[i], vf);
    }
}

static inline float32x4_t mag_square_4(float complex* src) {
    float32x4x2_t c;
    float32x4_t r2, i2, m2;
    
    c = vld2q_f32((const float*)src);
    // real = c.val[0]
    // imag = c.val[1]
    r2 = vmulq_f32(c.val[0], c.val[0]);
    i2 = vmulq_f32(c.val[1], c.val[1]);
    m2 = vaddq_f32(r2, i2);
    return m2;
}

static inline float32x4_t avg_4(float32x4_t v, int i) {
    const float b1 = 0.5;
    const float a0 = 1 - b1;
    // load delay
    float32x4_t z1 = vld1q_f32(&z[i]);
    z1 = vmulq_n_f32(z1, b1);
    v = vmulq_n_f32(v, a0);
    v = vaddq_f32(v, z1);
    // store on z;
    vst1q_f32(&z[i], v);
    return v;
}

void dsp_mag_squared_neon(float *dst, float complex* src, int len) {
    float32x4_t m2;
    for (int i = 0; i < 1024; i+=4) {
        m2 = mag_square_4(&src[i]);
        m2 = avg_4(m2, i);
        vst1q_f32(&dst[i+1024], m2);
    }
    for (int i = 1024; i < len; i+=4) {
        m2 = mag_square_4(&src[i]);
        m2 = avg_4(m2, i);
        vst1q_f32(&dst[i-1024], m2);
    }
}


void dsp_agc2_init(agc2_t *a) {
    a->ref = 1.0;
    a->rate = 0.005;
    a->gain = 1.0;
}


void dsp_agc(float complex *dst, float complex *src, int len) {
    static const float ref = 1.0;
    static const float rate = 0.05;
    static float gain = 1.0;

    float pow = 0.;
    for (int i = 0; i < len; i++) {
        dst[i] = src[i] * gain;
        // estimate power in output
        pow = crealf(dst[i] * conjf(dst[i]));
        gain += (ref - pow) * rate;
    }
}
