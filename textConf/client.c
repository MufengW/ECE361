#include "utils.h"

void get_input(char *buf);
static void process_msg(struct message *msg, char *buf);
enum type get_type(char *first_word);
void get_and_process_prompt(struct message *msg);
void connect_to_server(char *server_ip, char *server_port, int *sockfd);

void do_login(struct message *msg);

int main() {
    bool exit = false;
    struct message *msg = (struct message *)malloc(sizeof(struct message));
    while(!exit) {
        get_and_process_prompt(msg);
    }
    return 0;
}

void get_input(char *buf) {
    printf("Input your prompt below:\n");
    fgets(buf, MAX_DATA, stdin);
}

static void process_msg(struct message *msg, char *buf) {
    char *data = strdup(buf); // make a copy of original data to avoid overwrite
    if(strlen(data) == 0) {
        printf("data empty, try input again!\n");
        get_and_process_prompt(msg);
        return;
    }
    char delim[] = " \n\t\v\f\r";
    char *first_word = strtok(data, delim);
    if(first_word == NULL) {
        printf("data empty, try input again!\n");
        get_and_process_prompt(msg);
        return;
    }
    msg->msg_type = get_type(first_word);
    switch (msg->msg_type) {
        case LOGIN: {
            do_login(msg);
            break;
        }
        case EXIT: {
            break;
        }
        case JOIN: {
            break;
        }
        case LEAVE_SESS: {
            break;
        }
        case QUERY: {
            break;
        }
        case QUIT: {
            break;
        }
        case MESSAGE: {
            break;
        }
        default: {
            break;
        }
        return;
    }
}

void get_and_process_prompt(struct message *msg) {
    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);
    get_input(buf);
    process_msg(msg,buf);
}

enum type get_type(char *first_word) {
    if(strcmp(first_word, "/login") == 0) return LOGIN;
    if(strcmp(first_word, "/logout") == 0) return EXIT;
    if(strcmp(first_word, "/joinsession") == 0) return JOIN;
    if(strcmp(first_word, "/leavesession") == 0) return LEAVE_SESS;
    if(strcmp(first_word, "/list") == 0) return QUERY;
    if(strcmp(first_word, "/quit") == 0) return QUIT;

    return MESSAGE;
}

void connect_to_server(char *server_ip, char *server_port, int *sockfd) {
    struct addrinfo hints, *servinfo;
    char s[INET_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints. ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(server_ip, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    if((*sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("client: socket");
        exit(1);
    }
    if(connect(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(*sockfd);
        perror("client: connect");
        exit(1);
    }
    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr), s, sizeof(s));
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo);
}

void do_login(struct message *msg) {
    char delim[] = " \n\t\v\f\r";
    char *client_id = strtok(NULL, delim);
    char *password = strtok(NULL, delim);
    char *server_ip = strtok(NULL, delim);
    char *server_port = strtok(NULL, delim);
    if(server_port == NULL) {
        printf("\nlogin format error, please login with command:\n/login <client ID> <password> <server-IP> <server-port>\n\n");
        get_and_process_prompt(msg);
        return;
    }
    char *extra_input = strtok(NULL, delim);
    if(extra_input) {
        printf("extra input detected and ignored...\n");
    }

    int sockfd;
    connect_to_server(server_ip, server_port, &sockfd);

    // message data is the password
    memcpy(msg->data, password, strlen(password));
    msg->size = strlen((const char *)msg->data);
    memcpy(msg->source, client_id, strlen(client_id));

    send_message(msg, sockfd);

    recv_message(msg, sockfd);

    switch(msg->msg_type) {
        case LO_ACK: {
        printf("login successful!\n");
        return;
    }
    case LO_NAK: {
            printf("%s", msg->data);
        break;
    }
    default: {
        break;
     }
    }
    get_and_process_prompt(msg);

    //close(sockfd);
}
