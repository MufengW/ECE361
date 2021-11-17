#include "utils.h"

bool login = false;
int in_session_count = 0;
char *current_client = "";
char *current_session = "";
int sockfd;

void get_input(char *buf);
void detect_extra_input();
static void process_msg(struct message *msg, char *buf);
enum type get_type(char *first_word);
void get_and_process_prompt(struct message *msg);
void connect_to_server(char *server_ip, char *server_port, int *sockfd);

static void do_login(struct message *msg);
static void do_logout(struct message *msg);
static void do_newsession(struct message *msg);

int main() {
    bool exit = false;
    struct message *msg = (struct message *)malloc(sizeof(struct message));
    while(!exit) {
        get_and_process_prompt(msg);
    }
    return 0;
}

void get_input(char *buf) {
    if(!login) {
        printf("\nInput your prompt below:\n\n>> ");
    } else {
        printf("\n%s:\n>> ", current_client);
    }
    fgets(buf, MAX_DATA, stdin);
}

static void process_msg(struct message *msg, char *buf) {
    char *data = strdup(buf); // make a copy of original data to avoid overwrite
    if(strlen(data) == 0) {
        printf("\ndata empty, try input again!\n\n");
        get_and_process_prompt(msg);
        return;
    }
    char delim[] = " \n\t\v\f\r";
    char *first_word = strtok(data, delim);
    if(first_word == NULL) {
        printf("\ndata empty, try input again!\n\n");
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
            do_logout(msg);
            break;
        }
        case JOIN: {
            break;
        }
        case LEAVE_SESS: {
            break;
        }
        case NEW_SESS: {
            do_newsession(msg);
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
    if(strcmp(first_word, "/createsession") == 0) return NEW_SESS;
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
    //printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo);
}

static void do_login(struct message *msg) {
    if(login) {
        printf("\nyou have already logged in to %s on this machine!\n\n", current_client);
        get_and_process_prompt(msg);
        return;
    }
    char delim[] = " \n\t\v\f\r";
    char *client_id = strtok(NULL, delim);
    char *password = strtok(NULL, delim);
    char *server_ip = strtok(NULL, delim);
    char *server_port = strtok(NULL, delim);
    if(server_port == NULL) {
        printf("\nlogin format error, please login with command:\n\n/login <client ID> <password> <server-IP> <server-port>\n\n");
        get_and_process_prompt(msg);
        return;
    }

    detect_extra_input();

    connect_to_server(server_ip, server_port, &sockfd);

    // message data is the password
    memset(msg->data, 0, MAX_DATA);
    memcpy(msg->data, password, strlen(password));
    msg->size = strlen((const char *)msg->data);
    memset(msg->source, 0, MAX_NAME);
    memcpy(msg->source, client_id, strlen(client_id));

    send_message(msg, sockfd);
    recv_message(msg, sockfd);

    switch(msg->msg_type) {
        case LO_ACK: {
            printf("\nlogin successful!\n\n");
            login = true;
            current_client = (char *)msg->source;
            return;
        }
        case LO_NAK: {
            printf("%s", msg->data);
            break;
        }
        default: {
            ereport("unkown message type!");
            break;
         }
    }
    get_and_process_prompt(msg);

    //close(sockfd);
}

static void do_logout(struct message *msg) {
    if(!login) {
        printf("\nyou have not logged in to any account\n\n");
        get_and_process_prompt(msg);
        return;
    }
    detect_extra_input();

    memset(msg->source, 0, MAX_NAME);
    memcpy(msg->source, current_client, strlen(current_client));
    send_message(msg, sockfd);
    if(close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
    printf("\naccount %s have successfully logged out!\n\n", current_client);
    current_client = "";
    login = false;
}

static void do_newsession(struct message *msg) {
    if(in_session_count > 0) {
        printf("\nyou are already in session %s!\n\n", current_session);
        get_and_process_prompt(msg);
        return;
    }
    if(!login) {
        printf("\nyou need to login first!\n\n");
        get_and_process_prompt(msg);
        return;
    }
    char delim[] = " \n\t\v\f\r";
    char *session_id = strtok(NULL, delim);

    detect_extra_input();

    memset(msg->source, 0, MAX_NAME);
    memcpy(msg->source, current_client, strlen(current_client));

    memset(msg->data, 0, MAX_DATA);
    memcpy(msg->data, session_id, strlen(session_id));

    send_message(msg, sockfd);
    recv_message(msg, sockfd);

    switch(msg->msg_type) {
        case NS_ACK: {
            printf("\nnew session %s created!\n\n", session_id);
        ++in_session_count;
            current_session = session_id;
            return;
        }
        case NS_NAK: {
            printf("%s", msg->data);
            break;
        }
        default: {
            ereport("unkown message type!");
            break;
         }
    }
    get_and_process_prompt(msg);

}

void detect_extra_input() {
    char delim[] = " \n\t\v\f\r";
    char *extra_input = strtok(NULL, delim);
    if(extra_input) {
        printf("\nextra input dected and ignored...\n\n");
    }
}
