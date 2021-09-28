#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define FTP_STR "ftp"
#define YES "yes"
#define BUFF_LEN 1024

int main(int argc, char *argv[]) {
    char *address;
    int port;
    if (argc == 3) {
        address = argv[1];
        port = atoi(argv[2]);
    } else {
        printf("Usage: deliver <server address> <server port number>. \n");
        return 0;
    }
    char ftp_cmd[3];
    char file_name[256];
    printf("Enter your message in the format of \'ftp <file name>\': ");
    scanf("%s %s", ftp_cmd, file_name);

    if (strcmp(ftp_cmd, FTP_STR) != 0) {
        printf("%s: Command not found.\n", ftp_cmd);
        return 0;
    }

    FILE *file;
    if ((file = fopen(file_name, "r")) == NULL) {
        printf("File does not exist. \n");
        return 0;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(address, &server_addr.sin_addr.s_addr);

    socklen_t server_addr_len = sizeof (server_addr);

    sendto(sockfd, FTP_STR, sizeof (FTP_STR), MSG_CONFIRM,
            (struct sockaddr *) &server_addr, server_addr_len);

    char buf[BUFF_LEN];
    recvfrom(sockfd, (char*) buf, sizeof (buf), MSG_WAITALL,
            (struct sockaddr*) &server_addr, &server_addr_len);

    if (strcmp(buf, YES) == 0) {
        printf("A file transfer can start.\n");
    }

    fclose(file);
    return 0;
}
