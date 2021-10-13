#include <stdio.h>
#include "packet.h"

void serializePacket(const Packet *packet, char *result){
    printf("serializePacket\n");
}
void deserializePacket(const char* str, Packet *packet){
    printf("deserializePacket\n");    
}

void packetsToFile(const Packet *packet[], FILE *pFile){    
    printf("packetToFile\n");    
}
void fileToPackets(const FILE *pFile, Packet *packet[]){    
    printf("fileToPacket\n");    
}
