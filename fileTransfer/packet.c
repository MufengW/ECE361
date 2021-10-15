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

void serializePacket(const Packet *packet, char *result) {
    sprintf(result, "%d", packet->total_frag);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%d", packet->frag_no);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%d", packet->size);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%s", packet->filename);
    sprintf(result + strlen(result), "%c", DELIMITER);
    memcpy(result + strlen(result), packet -> filedata, DATA_SIZE * sizeof (char));
}

void deserializePacket(const char* str, Packet *packet) {
    char temp[BUFF_SIZE] = "";
    int start_index = 0;
    int end_index = 0;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    packet->total_frag = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    packet->frag_no = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    packet->size = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    packet->filename = malloc(sizeof (temp));
    sprintf(packet->filename, "%s", temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    memcpy(packet->filedata, &str[start_index], packet->size);
}

void packetsToFile(const Packet *packet[], char *pFile) {
    pFile = packet[0]->filename;
    int fd = creat(pFile, S_IRWXU);
    if (fd < 0) {
        printf("creat: %s: %s\n", pFile, strerror(errno));
        exit(1);
    }

    int packet_no = packet[0]->total_frag;
    int seg_size = 0;
    int i = 0, write_rt = 0;
    char buf[DATA_SIZE];
    memset(buf, 0, DATA_SIZE * sizeof (char));
    for (i = 0; i < packet_no; ++i) {
        Packet *tmp_pkt = (Packet*) packet[i];
        assert(packet_no == tmp_pkt->total_frag);
        assert(strcmp(pFile,tmp_pkt->filename)==0);
        seg_size = tmp_pkt->size;
        memcpy(buf, tmp_pkt->filedata, seg_size);
        write_rt = write(fd, buf, seg_size);
        if (write_rt < 0) {
            printf("write: %s: %s\n", pFile, strerror(errno));
            exit(1);
        }

    }
    assert(write_rt <= DATA_SIZE);

    if (close(fd) < 0) {
        printf("close: %s: %s\n", pFile, strerror(errno));
        exit(1);
    }
}

void fileToPackets(const char *pFile, Packet *packet[]) {
    int fd = open(pFile, O_RDONLY);
    if (fd < 0) {
        printf("open: %s: %s\n", pFile, strerror(errno));
        exit(1);
    }

    char buf[DATA_SIZE];
    memset(buf, 0, DATA_SIZE * sizeof (char));
    int file_size = get_file_size((char*) pFile);
    int packet_no = file_size / DATA_SIZE + 1;
    int i = 0, read_rt = 0;
    for (i = 0; i < packet_no; ++i) {
        read_rt = read(fd, buf, DATA_SIZE);
        if (read_rt < 0) {
            printf("read: %s: %s\n", pFile, strerror(errno));
            exit(1);
        }
        packet[i] = (Packet*) malloc(sizeof (Packet));
        Packet *tmp_pkt = packet[i];
        tmp_pkt->total_frag = packet_no;
        tmp_pkt->frag_no = i;
        tmp_pkt->size = read_rt;
        tmp_pkt->filename = (char*) pFile;
        memcpy(tmp_pkt->filedata, buf, read_rt);
    }
    assert(read_rt <= DATA_SIZE);

    if (close(fd) < 0) {
        printf("close: %s: %s\n", pFile, strerror(errno));
        exit(1);
    }

}

void free_packet(Packet *packet[], int packet_no) {
    int i = 0;
    for (i = 0; i < packet_no; ++i) {
        packet[i]->filename = NULL;
        free(packet[i]);
        packet[i] = NULL;
    }
    free(packet);
}

int get_file_size(char* file) {
    int size;
    struct stat st;
    stat(file, &st);
    size = st.st_size;
    return size;
}
