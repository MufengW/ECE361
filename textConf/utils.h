#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define __STR(n) #n
#define STR(n) __STR(n)

#define ereport(emessage) do {                                \
    fprintf(stderr,"%s:%d:\t%s:%s\n", __FILE__, __LINE__, __FUNCTION__, emessage);    \
} while(0)

#define MAX_NAME 256
#define MAX_DATA 2048

#define DELIMITER ':'
#define BACKLOG 10
#define MAX_ONLINE 20

enum type {
    LOGIN,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    NEW_SESS,
    NS_ACK,
    MESSAGE,
    QUERY,
    QU_ACK,

    //new added
    QUIT
};

struct message {
    enum type msg_type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void start_listen(char *port, int *sockfd);
int accept_message(struct message *msg, int sockfd);
void *get_in_addr(struct sockaddr *sa);
void serialize(struct message *msg, char *buf);
void deserialize(struct message *result, char *buf);
void send_message(struct message *msg, int sockfd);
void recv_message(struct message *msg, int sockfd);

#endif /* UTILS_H */
