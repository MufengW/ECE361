#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "packet.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: server <UDP listen port>. \n");
        exit(1);
    }
    char *port;
    port = argv[1];

    struct addrinfo hints, *servinfo;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    int rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1) {
        perror("listener: socket");
        exit(1);
    }
    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(sockfd);
        perror("listener: bind");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("Server receiving on port %s\n", port);

    char buf[BUFF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof (client_addr);

    if (recvfrom(sockfd, (char*) buf, sizeof (buf), MSG_WAITALL,
            (struct sockaddr *) &client_addr, &client_len) == -1) {
        perror("recvfrom");
        exit(1);
    }

    if (strcmp(buf, FTP_STR) == 0) {
        if (sendto(sockfd, YES, sizeof (YES), MSG_CONFIRM,
                (struct sockaddr *) &client_addr, client_len) == -1) {
            perror("server: sendto");
            exit(1);
        }
    } else {
        if (sendto(sockfd, NO, sizeof (NO), MSG_CONFIRM,
                (struct sockaddr *) &client_addr, client_len) == -1) {
            perror("server: sendto");
            exit(1);
        }
    }


    // recv string
    char serializedPacket[BUFF_SIZE];
    recvMsg(sockfd, client_addr, serializedPacket);
    printf("%s\n", serializedPacket);

    //recvfrom(... serializedPacket ...) in a loop
    Packet* packet = (Packet*) malloc(sizeof (Packet));
    deserializePacket(serializedPacket, packet);
    int packet_no = packet->total_frag;
    char* pFile = packet->filename;
    Packet **p = (Packet**) malloc(sizeof (Packet*) * packet_no);

    p[0] = packet;

    sendMsg(sockfd, ACK, client_addr);
    for (int i = 1; i < packet_no; ++i) {
        recvMsg(sockfd, client_addr, serializedPacket);
        printf("%s\n", serializedPacket);
        deserializePacket(serializedPacket, packet);
        p[i] = packet;
        sendMsg(sockfd, ACK, client_addr);
    }

    packetsToFile((const Packet**) p, pFile);
    
    printf("Finished writing file\n");
    free_packet(p, packet_no);
    return 0;
}
