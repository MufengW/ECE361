#include "utils.h"
#define BACKLOG 10

void start_connection(char *port, int *sockfd);

int main(int argc, char *argv[]) {
    struct sockaddr_storage; // connector's address information

    if(argc != 2) {
        printf("usage: server <TCP port number to listen on>\n");
        exit(1);
    }
    char *port = argv[1];

    int sockfd;
    start_connection(port, &sockfd);

    char buf[MAX_DATA];
    while(1) {
        recv_message(sockfd, buf);
    }
    return 0;
}

void start_connection(char *port, int *sockfd) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if(rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    *sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(*sockfd == -1) {
        perror("server: socket");
        exit(1);
    }
    if(bind(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(*sockfd);
        perror("server: bind");
        exit(1);
    }
    freeaddrinfo(servinfo);

    printf("server reciving on port %s\n", port);

    if(listen(*sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");
}
