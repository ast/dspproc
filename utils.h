//
//  utils.h
//  dspproc
//
//  Created by Albin Stigö on 18/12/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <sys/un.h>

int create_socket(struct sockaddr_un *client_addr);

#endif /* utils_h */
