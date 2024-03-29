#include "utils.h"

void start_listen(char *port, int *sockfd) {
    struct addrinfo hints, *servinfo;
    int yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if(rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    *sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(*sockfd == -1) {
        perror("server: socket");
        exit(1);
    }
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(bind(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(*sockfd);
        perror("server: bind");
        exit(1);
    }
    freeaddrinfo(servinfo);

    printf("server reciving on port %s\n", port);

    if(listen(*sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("waiting for connections...\n");
}


void *get_in_addr(struct sockaddr *sa) {
    assert(sa->sa_family == AF_INET);
    return &(((struct sockaddr_in *)sa)->sin_addr);
}

void serialize(struct message *msg, char *result) {
    int source_len = strlen((const char *)msg->source);
    int data_len = strlen((const char *)msg->data);
    // serialized_data structure: 
    // <source_len> : <data_len> : <msg_type> : <size> : <source> <data>
    sprintf(result, "%d", source_len);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%d", data_len);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%d", msg->msg_type);
    sprintf(result + strlen(result), "%c", DELIMITER);
    sprintf(result + strlen(result), "%d", msg->size);
    sprintf(result + strlen(result), "%c", DELIMITER);
    memcpy(result + strlen(result), msg->source, source_len * sizeof(char));
    memcpy(result + strlen(result), msg->data, data_len * sizeof(char));
}

void deserialize(struct message *result, char *str) {
    char temp[MAX_DATA] = "";
    int start_index = 0;
    int end_index = 0;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    int source_len = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    int data_len = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    result->msg_type = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    end_index = (int) (strchr(str + start_index, DELIMITER) - str);
    memcpy(temp, &str[start_index], end_index - start_index);
    result->size = atoi(temp);
    memset(temp, 0, sizeof (temp));
    start_index = end_index + 1;

    memcpy(result->source, &str[start_index], source_len);
    memcpy(result->data, &str[start_index + source_len], data_len);
}

int accept_conn(int sockfd) {
    char s[INET_ADDRSTRLEN];

    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if(new_fd == -1) {
        perror("accept");
        exit(1);
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
    printf("\ngot connection from %s...\n", s);
    fflush(stdout);
    return new_fd;
}

void send_message(struct message *msg, int sockfd) {
    char buf[sizeof(struct message) + sizeof(int) * 2];
    memset(buf, 0, sizeof(buf));
    serialize(msg, buf);
    if(send(sockfd, buf, MAX_DATA, 0) == -1) {
        perror("send");
        exit(1);
    }
//    printf("send, fd = %d\nmsg = %s\n", sockfd, msg->data);
    fflush(stdout);
}

bool recv_message(struct message *msg, int sockfd) {
    memset(msg, 0, sizeof(struct message));
    char buf[sizeof(struct message) + sizeof(int) * 2];
    int rt = recv(sockfd, buf, MAX_DATA, 0);
    if(rt == -1) {
        perror("recv");
        exit(1);
    }
    if(rt > 0) {
        deserialize(msg, buf);
    }
//    printf("recv, fd = %d\n", sockfd);
    return (rt != 0);
}

void set_str_val(char* src, char *dst) {
    char *tmp = strdup(dst);
    memset(src, 0, strlen(src));
    memcpy(src, tmp, strlen(tmp));
}
