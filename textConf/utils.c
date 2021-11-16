#include "utils.h"

void *get_in_addr(struct sockaddr *sa) {
    assert(sa->sa_family == AF_INET);
    return &(((struct sockaddr_in *)sa)->sin_addr);
}

void serialize(struct message *msg, char *result) {
    int source_len = strlen((const char *)msg->source);
    int data_len = strlen((const char *)msg->data);
    // serialized_data structure: 
    // <source_len> <data_len> <msg_type> <size> <source> <data>
    // no delimiter needed
    sprintf(result, "%d", source_len);
    sprintf(result + strlen(result), "%d", data_len);
    sprintf(result + strlen(result), "%d", (int)msg->msg_type);
    sprintf(result + strlen(result), "%d", (int)msg->size);
    memcpy(result + strlen(result), msg->source, source_len * sizeof(char));
    memcpy(result + strlen(result), msg->data, data_len * sizeof(char));
}

void deserialize(struct message *result, char *str) {
    int source_len = str[0] - '0';
    int data_len = str[1] - '0';

    result->msg_type = str[2] - '0';

    result->size = str[3] - '0';

    memcpy(result->source, str + 4, source_len);

    memcpy(result->data, str + 4 + source_len, data_len);
}

void send_message(int sockfd, char *serialized_data) {
    if(send(sockfd, serialized_data, MAX_DATA, 0) == -1) {
        perror("send");
        exit(1);
    }
}

void recv_message(int sockfd, char *buf) {
    char s[INET_ADDRSTRLEN];

    memset(buf, 0, MAX_DATA);
    struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1) {
            perror("accept");
            exit(1);
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);
        if(recv(new_fd, buf, MAX_DATA, 0) == -1) {
            perror("recv");
            exit(1);
        }
        printf("%s\n",buf);
        struct message *msg = (struct message *)malloc(sizeof(struct message));
        deserialize(msg, buf);
        printf("client id: %s\npassword: %s\n", msg->source, msg->data);
        close(new_fd);

}
