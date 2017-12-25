//
//  utils.c
//  dspproc
//
//  Created by Albin Stigö on 18/12/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

static const int N = 2048;

int create_socket(struct sockaddr_un *client_addr) {
    int sd = 0;
    ssize_t err = 0;
    static const int send_buf_periods = 4;
    
    sd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    client_addr->sun_family = AF_UNIX;
    strcpy(client_addr->sun_path, "/tmp/sdrclient");
    
    int send_buf_size = N * sizeof(float) * send_buf_periods;
    err = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size));
    if(err < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    return sd;
}
