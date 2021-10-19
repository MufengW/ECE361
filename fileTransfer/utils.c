#include "utils.h"

bool rand_seed_gen = false;

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

double uniform_rand() {
    if(!rand_seed_gen) {
        rand_seed_gen = true;
        srand((unsigned)time(NULL));
    }
    return((double)rand() / (double)RAND_MAX);
}
