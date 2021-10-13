#ifndef PACKET_H
#define PACKET_H

#define DATA_SIZE 1000

typedef struct {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[DATA_SIZE];
} Packet;

void serializePacket(const Packet *packet, char *result);
void deserializePacket(const char* str, Packet *packet);

#endif /* PACKET_H */

