#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "packet.h"

void sendMsg();
bool recvMsg();

double uniform_rand();

#define MILLISEC_PER_SEC 1000

#endif /* UTILS_H */
