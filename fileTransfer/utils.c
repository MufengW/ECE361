//#include "utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "packet.h"

void sendMsg(int sockfd, const void* msg, struct sockaddr_in *addr) {
    socklen_t addr_len = sizeof (*addr);
    if (sendto(sockfd, msg, BUFF_SIZE, MSG_CONFIRM,
            (struct sockaddr *) addr, addr_len) == -1) {
        perror("sendto");
        exit(1);
    }
}

void recvMsg(int sockfd, struct sockaddr_in *addr, char* buf) {
    socklen_t addr_len = sizeof (*addr);
    if (recvfrom(sockfd, (char*) buf, BUFF_SIZE, MSG_WAITALL,
            (struct sockaddr*) addr, &addr_len) == -1) {
        perror("recvfrom");
        exit(1);
    }

}
