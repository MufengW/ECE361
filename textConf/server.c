#include "utils.h"

enum account_stat {
    VALID,
    ACCOUNT_NOT_EXIST,
    WRONG_PASSWORD,
    ALREADY_ONLINE,
    NO_MORE_SPACE
};

const char *account_not_exist = "\nthis account does not exist!\n\n";
const char *wrong_password = "\nwrong password!\n\n";
const char *already_online = "\nthis account has alreay logged in on another machine!\n\n";
const char *no_more_space = "\nto many users online, need to wait for someone to logout!\n\n";

char *online_client[MAX_ONLINE];
int online_count = 0;

static bool process_message(struct message *msg, int sockfd);
static void do_login(struct message *msg, int sockfd);
static void do_logout(struct message *msg, int sockfd);
void add_account(char *client_id);
void remove_account(char *client_id);
int find_client(char *client_id);
static enum account_stat check_account(char *client_id, char *password);

int main(int argc, char *argv[]) {
    struct sockaddr_storage; // connector's address information

    if(argc != 2) {
        printf("\nusage: server <TCP port number to listen on>\n\n");
        exit(1);
    }
    char *port = argv[1];

    int listen_sockfd;
    start_listen(port, &listen_sockfd);

    while(1) {
        struct message *msg = (struct message*)malloc(sizeof(struct message));

        int recv_sockfd = accept_message(msg, listen_sockfd);
        bool exit = false;
        while(!exit) {
            recv_message(msg, recv_sockfd);
            exit = process_message(msg, recv_sockfd);
            free(msg);
        }
    }
    return 0;
}

static bool process_message(struct message *msg, int sockfd) {
    switch (msg->msg_type) {
        case LOGIN: {
            do_login(msg, sockfd);
            return false;
        }
        case EXIT: {
            do_logout(msg, sockfd);
        return true;
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
    }
    return false;
}

static void do_login(struct message *msg, int sockfd) {
    char *client_id = (char *)msg->source;
    char *password = (char *)msg->data;
    enum account_stat stat = check_account(client_id, password);
    switch(stat) {
        case VALID: {
            msg->msg_type = LO_ACK;
            add_account(client_id);
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
        case ALREADY_ONLINE: {
            msg->msg_type = LO_NAK;
            memcpy(msg->data, already_online, strlen(already_online));
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

static void do_logout(struct message *msg, int sockfd) {
    char *client_id = (char *)msg->source;
    remove_account(client_id);
    if(close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
}

void add_account(char *client_id) {
    for(int i = 0; i < MAX_ONLINE; ++i) {
        if(online_client[i] == NULL) {
            online_client[i] = strdup(client_id);
            ++online_count;
            return;
        }
    }
    ereport("no more space!");
}

void remove_account(char *client_id) {
    int idx = find_client(client_id);
    if(idx == MAX_ONLINE) {
        ereport("client to remove not found!");
    }
    online_client[idx] = "";
    printf("\nclient %s removed\n\n", client_id);
    --online_count;
}

int find_client(char *client_id) {
    for(int i = 0; i < MAX_ONLINE; ++i) {
        char *temp_id = online_client[i];
        if(temp_id != NULL && strcmp(client_id, temp_id) == 0) {
            return i;
        }
    }
    return MAX_ONLINE;
}

static enum account_stat check_account(char *client_id, char *password) {
    if(find_client(client_id) < MAX_ONLINE) return ALREADY_ONLINE;
        if(online_count + 1 == MAX_ONLINE) return NO_MORE_SPACE;
    return VALID;
}
