#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define FTP_STR "ftp"
#define YES "yes"
#define NO "no"
#define BUFF_LEN 1024

int main(int argc, char *argv[]) {
    char *port;
    if (argc == 2) {
        port = argv[1];
    } else {
        printf("Usage: server <UDP listen port>. \n");
        return 0;
    }

    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    printf("Server receiving on port %s\n", port);

    char buf[BUFF_LEN];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof (client_addr);

    recvfrom(sockfd, (char*) buf, sizeof (buf), MSG_WAITALL,
            (struct sockaddr *) &client_addr, &client_len);

    if (strcmp(buf, FTP_STR) == 0) {
        sendto(sockfd, YES, sizeof (YES), MSG_CONFIRM,
                (struct sockaddr *) &client_addr, client_len);
    } else {
        sendto(sockfd, NO, sizeof (NO), MSG_CONFIRM,
                (struct sockaddr *) &client_addr, client_len);
    }
    return 0;
}
