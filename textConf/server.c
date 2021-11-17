#include "utils.h"

enum account_stat {
    VALID,
    ACCOUNT_NOT_EXIST,
    WRONG_PASSWORD,
    NO_MORE_SPACE
};

const char *account_not_exist = "the account does not exist!\n";
const char *wrong_password = "wrong password!\n";
const char *no_more_space = "to many users online, need to wait for someone logout!\n";

char *online_client[MAX_ONLINE];
int online_count = 0;

static void process_message(struct message *msg, int sockfd);
static void do_login(struct message *msg, int sockfd);
static enum account_stat check_account(char *client_id, char *password);

int main(int argc, char *argv[]) {
    struct sockaddr_storage; // connector's address information

    if(argc != 2) {
        printf("usage: server <TCP port number to listen on>\n");
        exit(1);
    }
    char *port = argv[1];

    int listen_sockfd;
    start_listen(port, &listen_sockfd);

    while(1) {
        struct message *msg = (struct message*)malloc(sizeof(struct message));

        int recv_sockfd = accept_message(msg, listen_sockfd);
    recv_message(msg, recv_sockfd);
        process_message(msg, recv_sockfd);
    free(msg);
    }
    return 0;
}

static void process_message(struct message *msg, int sockfd) {
    switch (msg->msg_type) {
        case LOGIN: {
            do_login(msg, sockfd);
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

static void do_login(struct message *msg, int sockfd) {
    char *client_id = (char *)msg->source;
    char *password = (char *)msg->data;
    enum account_stat stat = check_account(client_id, password);
    switch(stat) {
        case VALID: {
            msg->msg_type = LO_ACK;
        online_client[online_count] = strdup(client_id);
        ++online_count;
            break;
        }
        case ACCOUNT_NOT_EXIST: {
            msg->msg_type = LO_NAK;
        memcpy(msg->data, account_not_exist, strlen(account_not_exist));
            break;
        }
        case WRONG_PASSWORD: {
            msg->msg_type = LO_NAK;
        memcpy(msg->data, wrong_password, strlen(wrong_password));
            break;
        }
    case NO_MORE_SPACE: {
            msg->msg_type = LO_NAK;
        memcpy(msg->data, no_more_space, strlen(no_more_space));
        break;
    }
        default: {
            break;
        }
    }
    send_message(msg, sockfd);
}

static enum account_stat check_account(char *client_id, char *password) {
    if(online_count + 1 == MAX_ONLINE) return NO_MORE_SPACE;
    return VALID;
}
