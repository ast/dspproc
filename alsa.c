//
//  alsa.c
//  fcdsdr
//
//  Created by Albin Stigö on 21/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include "alsa.h"

/*void sdr_pcm_set_swparms(snd_pcm_t *pcm_handle) {
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    
    if (snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams, 2048 * 3) < 0) {
        perror("snd_pcm_sw_params_set_start_threshold");
        exit(EXIT_FAILURE);
    }
}*/

/* Try to get an ALSA capture handle */
snd_pcm_t* sdr_pcm_handle(const char* pcm_name, snd_pcm_uframes_t frames, snd_pcm_stream_t stream) {
    snd_pcm_t *pcm_handle;
    //snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
    snd_pcm_hw_params_t *hwparams;
    
    const unsigned int rate = 39000;      // Fixed sample rate of VFZSDR.
    const unsigned int periods = 4;       // Number of periods in ALSA ringbuffer.
    
    snd_pcm_hw_params_alloca(&hwparams);
    
    /* Open normal blocking */
    if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
        fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
        exit(EXIT_FAILURE);
    }
    
    /* Init hwparams with full configuration space */
    if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
        fprintf(stderr, "Can not configure this PCM device.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Interleaved access. (IQ interleaved). */
    if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        fprintf(stderr, "Error setting access.\n");
        exit(EXIT_FAILURE);
    }
    
    unsigned int channels = 2;
    /* Set number of channels */
    if (snd_pcm_hw_params_set_channels_near(pcm_handle, hwparams, &channels) < 0) {
        fprintf(stderr, "Error setting channels.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set sample format */
    /* Use native format of device to avoid costly conversions */
    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S32_LE) < 0) {
        fprintf(stderr, "Error setting format.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set sample rate. If the exact rate is not supported exit */
    if (snd_pcm_hw_params_set_rate(pcm_handle, hwparams, rate, 0) < 0) {
        fprintf(stderr, "Error setting rate.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set number of periods. Periods used to be called fragments. */
    if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
        fprintf(stderr, "Error setting periods.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set buffer size (in frames). The resulting latency is given by */
    /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (frames * periods)) < 0) {
        fprintf(stderr, "Error setting buffersize.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
        fprintf(stderr, "Error setting HW params.\n");
        exit(EXIT_FAILURE);
    }
    
    return pcm_handle;
}


