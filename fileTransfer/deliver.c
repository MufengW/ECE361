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

#define FTP_STR "ftp"
#define YES "yes"
#define BUFF_LEN 1024

int main(int argc, char *argv[]) {
    char *address;
    int port;
    if (argc != 3) {
        printf("Usage: deliver <server address> <server port number>. \n");
        exit(1);
    }
    address = argv[1];
    port = atoi(argv[2]);

    char ftp_cmd[3];
    char file_name[256];

    printf("Enter your message in the format of \'ftp <file name>\': \n");

    scanf("%s %s", ftp_cmd, file_name);

    if (strcmp(ftp_cmd, FTP_STR) != 0) {
        printf("%s: Command not found.\n", ftp_cmd);
        exit(1);
    }

    int src_fd = 0;
    src_fd = open(file_name, O_RDONLY);
    if(src_fd < 0) {
        printf("open: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }
    if(close(src_fd) < 0) {
        printf("close: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }

    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("talker: socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(address, &server_addr.sin_addr);

    socklen_t server_addr_len = sizeof (server_addr);

    if(sendto(sockfd, FTP_STR, sizeof (FTP_STR), MSG_CONFIRM,
            (struct sockaddr *) &server_addr, server_addr_len) == -1) {
        perror("client: sendto");
        exit(1);
    }


    char buf[BUFF_LEN];
    if(recvfrom(sockfd, (char*) buf, sizeof (buf), MSG_WAITALL,
            (struct sockaddr*) &server_addr, &server_addr_len) == -1) {
        perror("client: recvfrom");
        exit(1);
    }

    if (strcmp(buf, YES) == 0) {
        printf("A file transfer can start.\n");
    }

    return 0;
}
