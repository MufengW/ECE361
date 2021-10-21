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

    recvMsg(sockfd, &client_addr, buf);

    if (strcmp(buf, FTP_STR) == 0) {
        sendMsg(sockfd, YES, &client_addr);
        printf("A file transfer can start.\n");
    } else {
        sendMsg(sockfd, NO, &client_addr);
        printf("%s: Command not found.\n", buf);
        exit(1);
    }

    // recv string
    char serializedPacket[BUFF_SIZE];
    recvMsg(sockfd, &client_addr, serializedPacket);

    Packet* packet = (Packet*) malloc(sizeof (Packet));
    deserializePacket(serializedPacket, packet);
    int packet_no = packet->total_frag;
    char* pFile = packet->filename;
    Packet **p = (Packet**) malloc(sizeof (Packet*) * packet_no);

    char segNum[SEGNUM_SIZE]; // segNum stores the next packet num in string, sent back to client as ACK
    p[0] = packet;

    int i = p[0]->frag_no; // i is the currently recieving packet num
    sprintf(segNum, "%d", i+1);
    sendMsg(sockfd, segNum, &client_addr);

    while (i+1 != packet_no) {
        recvMsg(sockfd, &client_addr, serializedPacket);
        if(uniform_rand() > 1e-2) { // 99% not drop
            Packet* tmp_packet = (Packet*)malloc(sizeof(Packet));
            deserializePacket(serializedPacket, tmp_packet);
            i = tmp_packet->frag_no; //update i to current packet frag_no
            p[i] = tmp_packet;

            //ready to recv next packet
            sprintf(segNum, "%d", i+1);
            sendMsg(sockfd, segNum, &client_addr);
        } else { // 1% drop
            printf("Packet %d dropped!\n",i+1);
        }
    }

    packetsToFile((const Packet**) p, pFile);

    printf("Finished writing file\n");
    free_packet(p, packet_no);
    return 0;
}
