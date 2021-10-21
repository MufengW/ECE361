#ifndef PACKET_H
#define PACKET_H

#define DATA_SIZE 10
#define BUFF_SIZE 2048
#define SEGNUM_SIZE 4
#define TIMEOUT_SEC 0
#define TIMEOUT_MICROSEC 10000
#define DELIMITER ':'

#define FTP_STR "ftp"
#define YES "yes"
#define NO "no"
#define ACK "ack"

typedef struct {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[DATA_SIZE];
} Packet;

void serializePacket(const Packet *packet, char *result);
void deserializePacket(const char* str, Packet *packet);
void packetsToFile(const Packet *packet[], char *pFile);
void fileToPackets(const char *pFile, Packet *packet[]);
void free_packet(Packet *packet[], int packet_no);

int get_file_size(char* file);
#endif /* PACKET_H */

