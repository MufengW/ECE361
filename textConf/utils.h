#ifndef UTILS_H
#define UTILS_H

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
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
    exit(1);                        \
} while(0)

#define MAX_NAME 256
#define MAX_DATA 2048

#define DELIMITER ':'
#define BACKLOG 10
#define MAX_ACCOUNT 20
#define MAX_SESSION 20

enum type {
    LOGIN = 0,
    EXIT,
    NEW_SESS,
    JOIN,
    LEAVE_SESS,
    QUERY,
    QUIT, // new added
    MESSAGE,
    AGAIN, // new added

    LO_ACK,
    LO_NAK,
    EXIT_DONE, // new added
    NS_ACK,
    NS_NAK,
    JN_ACK,
    JN_NAK,
    QU_ACK,
    MESSAGE_PRINT // new added
};

struct message {
    enum type msg_type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void start_listen(char *port, int *sockfd);
int accept_conn(int sockfd);
void *get_in_addr(struct sockaddr *sa);
void serialize(struct message *msg, char *buf);
void deserialize(struct message *result, char *buf);
void send_message(struct message *msg, int sockfd);
bool recv_message(struct message *msg, int sockfd);
void set_str_val(char* src, char *dst);
#endif /* UTILS_H */
