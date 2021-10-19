#include "utils.h"

int main(int argc, char *argv[]) {
    char *address;
    int port;
    if (argc != 3) {
        printf("Usage: deliver <server address> <server port number>. \n");
        exit(1);
    }
    address = argv[1];
    port = atoi(argv[2]);

    printf("Enter your message in the format of \'ftp <file name>\': \n");

    char prompt[256];
    fgets(prompt, 256, stdin);

    char delim[] = " \n\t\v\f\r";
    char *ftp_cmd = strtok(prompt, delim);
    if (strcmp(ftp_cmd, FTP_STR) != 0) {
        printf("%s: Command not found.\n", ftp_cmd);
        exit(1);
    }

    char *file_name = strtok(NULL, delim);
    if (!file_name) {
        printf("Please enter a file followed by 'ftp'.\n");
        exit(1);
    }
    char *extra_word = strtok(NULL, delim);
    if (extra_word) {
        printf("%s: extra input!\n", extra_word);
        exit(1);
    }
    int src_fd = 0;
    src_fd = open(file_name, O_RDONLY);
    if (src_fd < 0) {
        printf("open: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }
    if (close(src_fd) < 0) {
        printf("close: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("talker: socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(address, &server_addr.sin_addr);

    char buf[BUFF_SIZE];

    clock_t t;
    t = clock();
    sendMsg(sockfd, FTP_STR, &server_addr);

    recvMsg(sockfd, &server_addr, buf);

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC * MILLISEC_PER_SEC;
    printf("RRT: %f ms.\n",time_taken);

    if (strcmp(buf, YES) == 0) {
        printf("A file transfer can start.\n");
    } else {
        printf("Did not recieve ACK, abort...\n");
        exit(1);
    }

    int file_size = get_file_size((char*) file_name);
    int packet_no = file_size / DATA_SIZE + 1;
    Packet** packet = (Packet**) malloc(sizeof (Packet*) * packet_no);
    fileToPackets(file_name, packet);

    char serializedPacket[BUFF_SIZE];
    int i = 0;
    for (i = 0; i < packet_no; ++i) {
        serializePacket((const Packet*) packet[i], serializedPacket);
        sendMsg(sockfd, serializedPacket, &server_addr);
        recvMsg(sockfd, &server_addr, buf);
        while (strcmp(buf, ACK) != 0) {
            printf("Waiting for server to ACK packet %d/%d\n", i, packet_no);
        }
    }
    printf("File transfer finished\n");
    free_packet(packet, packet_no);
    return 0;
}
