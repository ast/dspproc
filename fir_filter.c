//
//  fir_filter.c
//  cbtest
//
//  Created by Albin Stigö on 27/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include "fir_filter.h"
#include <arm_neon.h>

bool fir_init(fir_filter_t *f, float *h, uint16_t n) {
    cb_init(&f->cb, 65536/2);
    f->h = malloc(n * sizeof(float));
    f->n = n;
    
    // TODO reverse
    // copy taps
    for(int i = 0; i < n; i++) {
        f->h[i] = h[i];
    }
    
    // prime buffer
    cb_produce(&f->cb, (n - 1) * sizeof(float complex));
    
    return true;
}

void fir_destroy(fir_filter_t *f) {
    cb_destroy(&f->cb);
}

float complex* fir_push (fir_filter_t *f, uint32_t amount) {
    float complex *ptr = (float complex*) cb_writeptr(&f->cb);
    cb_produce(&f->cb, amount * sizeof(float complex));
    return ptr;
}

void fir_execute(fir_filter_t *f, float complex* dst, uint16_t len) {
    // put on circular buffer
    //int size = len * sizeof(float complex);
    //memcpy(cb_writeptr(&f->cb), (void*) src, size);
    //cb_produce(&f->cb, size);
    
    // for each sample put on buffer
    for (int n = 0; n < len; n++) {
        float zeros[4] = {0,0,0,0};
        float tmp[4];
        // init sum
        float32x4_t r_acc = vld1q_f32(zeros);
        float32x4_t i_acc = vld1q_f32(zeros);
        
        // for each tap 4 at a time
        for (int i = 0; i < f->n; i+=4) {
            const float complex *src = (float complex*)cb_readptr(&f->cb);
            // inner loop
            float32x4_t h = vld1q_f32(&f->h[i]);
            float32x4x2_t x = vld2q_f32((const float*)&src[i]);
            r_acc = vmlaq_f32(r_acc, x.val[0], h);
            i_acc = vmlaq_f32(i_acc, x.val[1], h);
        }
        
        // summing
        float32x2_t r_sum = vpadd_f32(vget_low_f32(r_acc), vget_high_f32(r_acc));
        float32x2_t i_sum = vpadd_f32(vget_low_f32(i_acc), vget_high_f32(i_acc));
        float32x4_t q_sum = vcombine_f32(r_sum, i_sum);
        vst1q_f32(tmp, q_sum);
        
        dst[n] = (tmp[0] + tmp[1]) + I * (tmp[2] + tmp[3]);
        // consume one
        cb_consume(&f->cb, sizeof(float complex));
    }
}
