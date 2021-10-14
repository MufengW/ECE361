#include <stdio.h>
#include "packet.h"
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void serializePacket(const Packet *packet, char *result){
    printf("serializePacket\n");
}
void deserializePacket(const char* str, Packet *packet){
    printf("deserializePacket\n");    
}

void packetsToFile(const Packet *packet[], char *pFile){
    printf("packetToFile\n");
    pFile = packet[0]->filename;
    int fd = creat(pFile,S_IRWXU);
    if(fd < 0) {
            printf("creat: %s: %s\n", pFile, strerror(errno));
            exit(1);
    }

    int packet_no = packet[0]->total_frag;
    int seg_size = 0;
    int i = 0, write_rt = 0;
    char buf[DATA_SIZE];
    memset(buf, 0, DATA_SIZE * sizeof(char));
    for(i = 0; i < packet_no; ++i) {
	    Packet *tmp_pkt = (Packet*)packet[i];
	    assert(packet_no == tmp_pkt->frag_no);
	    assert(pFile == tmp_pkt->filename);
	    seg_size = tmp_pkt->size;
	    memcpy(buf,tmp_pkt->filedata,seg_size);
	    write_rt = write(fd, buf, seg_size);
	    if(write_rt < 0) {
		    printf("write: %s: %s\n", pFile, strerror(errno));
		    exit(1);
	    }

    }
    assert(write_rt <= DATA_SIZE);

    if(close(fd) < 0) {
	    printf("close: %s: %s\n", pFile, strerror(errno));
            exit(1);
    }
}
void fileToPackets(const char *pFile, Packet *packet[]){
    printf("fileToPacket\n");
    int fd = open(pFile, O_RDONLY);
    if(fd < 0) {
	    printf("open: %s: %s\n", pFile, strerror(errno));
	    exit(1);
    }

    char buf[DATA_SIZE];
    memset(buf, 0, DATA_SIZE * sizeof(char));
    int file_size = get_file_size((char*)pFile);
    int packet_no = file_size / DATA_SIZE + 1;
    int i = 0, read_rt = 0;
    for(i = 0; i < packet_no; ++i) {
	    read_rt = read(fd, buf, DATA_SIZE);
	    if(read_rt < 0) {
		    printf("read: %s: %s\n", pFile, strerror(errno));
		    exit(1);
	    }
	    packet[i] = (Packet*)malloc(sizeof(Packet));
	    Packet *tmp_pkt = packet[i];
	    tmp_pkt->total_frag = packet_no;
	    tmp_pkt->frag_no = i;
	    tmp_pkt->size = read_rt;
	    tmp_pkt->filename = (char*)pFile;
	    memcpy(tmp_pkt->filedata, buf,read_rt);
    }
    assert(read_rt <=  DATA_SIZE);

    if(close(fd) < 0) {
	    printf("close: %s: %s\n", pFile, strerror(errno));
	    exit(1);
    }

}

void free_packet(Packet *packet[], int packet_no){
	int i = 0;
	for(i = 0; i<packet_no; ++i){
		packet[i]->filename = NULL;
		free(packet[i]);
		packet[i] = NULL;
	}
	free(packet);
}

int get_file_size(char* file){
	int size;
	struct stat st;
	stat(file, &st);
	size = st.st_size;
	return size;
}

//int main(int argc, char** argv){
//	char *file = argv[1];
//	int file_size = get_file_size(file);
//	int packet_no = file_size / DATA_SIZE + 1;
//	Packet ** p = malloc(sizeof(Packet * ) * packet_no);
//	fileToPackets(file,p);
//	packetsToFile((const Packet**)p,file);
//	return 0;
//}
