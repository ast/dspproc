#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <math.h>
#include <complex.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <liquid/liquid.h>
#include "arm_neon.h"
#include <NE10/NE10.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "utils.h"
#include "alsa.h"
#include "dsp.h"
#include "fir_filter.h"


void init_neon() {
    ne10_init();
    // Neon filter
    if(ne10_HasNEON() != NE10_OK) {
        perror("ne10_HasNEON");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    
    const int N = 2048;
    
    ssize_t err = 0;
    int sd = 0;
    struct sockaddr_un client_addr;
    memset(&client_addr, 0x00, sizeof(client_addr));
    
    sd = create_socket(&client_addr);
    
    // Low pass filter
    float fc = 5000.0 / 39000.0;         // filter cutoff frequency
    float ft = 0.005f;           // filter transition
    float As = 60.0f;           // stop-band attenuation [dB]
    float mu = 0.0f;            // fractional timing offset
    
    // Estimate required filter length and generate filter.
    unsigned int h_len = estimate_req_filter_len(ft,As)+2;
    printf("Filter len: %d\n", h_len);
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,As,mu,h);
    
    fir_filter_t fir;
    float complex out_test[N];
    fir_init(&fir, h, h_len);
    
    init_neon();
    ne10_fft_cfg_float32_t cfg;
    cfg = ne10_fft_alloc_c2c_float32(N);
    // Window
    float hann[N];
    for (int n = 0; n < N; n++) {
        hann[n]   = dsp_hann(n, N);
    }
    //printf("power = %f\n", dsp_power(hann, N));
    
    frame_t         iqframes_s32[N];
    frame_t         out_s32[N];
    float complex   iqframes_c32_win[N];
    
    float complex   fft_c32[N];
    //float           abs_fft_c32[N];
    //float           abs_fft_squared_c32[N];
    
    if (argc < 3) {
        fprintf(stderr, "to few args\n");
        abort();
    }
    
    snd_pcm_t* pcm_handle = sdr_pcm_handle(argv[1], N, SND_PCM_STREAM_CAPTURE);
    snd_pcm_t* pcm_pb_handle = sdr_pcm_handle(argv[2], N, SND_PCM_STREAM_PLAYBACK);
    
    // Zero buffers
    memset((void*)iqframes_s32, 0x00, sizeof(iqframes_s32));
    memset((void*)out_s32, 0x00, sizeof(out_s32));
    
    // Fill alsa ringbuffer with silence
    for (int i = 0; i < 4; i++) {
        snd_pcm_writei(pcm_pb_handle, (void*)iqframes_s32, N);
    }
    
    float y, x, xm1, ym1;
    //float complex tmpc[N];
    float fft_m2[N];
    
    agc2_t agc;
    dsp_agc2_init(&agc);
    
    snd_pcm_sframes_t n_frames = 0;
    snd_pcm_sframes_t out_frames = 0;
    while(true) {
        
        if (n_frames < 0) {
            snd_pcm_recover(pcm_handle, (int) n_frames, 0);
        }
        
        while ((n_frames = snd_pcm_readi(pcm_handle, (void*)iqframes_s32, N)) > 0) {
            float complex *iqframes_c32_2 = fir_push(&fir, N);
            dsp_i24_to_c(iqframes_c32_2, iqframes_c32_win, iqframes_s32, hann, N);
            fir_execute(&fir, out_test, N);
            
            ne10_fft_c2c_1d_float32((ne10_fft_cpx_float32_t*)fft_c32,
                                    (ne10_fft_cpx_float32_t*)iqframes_c32_win, cfg, 0);
            
            // Mag squared and avg
            dsp_mag_squared_neon(fft_m2, fft_c32, N);
            // Send to spectrum socket
            err = sendto(sd,
                         fft_m2,
                         N * sizeof(float),
                         MSG_DONTWAIT,
                         (struct sockaddr*) &client_addr,
                         sizeof(struct sockaddr));
            
            /*if(err < 0) {
             perror("send");
             }*/
            
            // AM
            // Loop over all samples
            for (int i = 0; i < N; i++) {
                float complex out = dsp_agc2(&agc, out_test[i]);
                x = cabsf(out);
                // remove DC
                y = x - xm1 + 0.995 * ym1;
                xm1 = x;
                ym1 = y;
                out_s32[i].l = (int16_t)(0.5 * y * 0x7fff) << 16;
            }
            
            if((out_frames = snd_pcm_writei(pcm_pb_handle, (void*)out_s32, N)) < 0) {
                snd_pcm_recover(pcm_pb_handle, (int)out_frames, 0);
            }
        }
    }
    
    // Cleanup.
    snd_pcm_close (pcm_handle);
    
    return 0;
}
