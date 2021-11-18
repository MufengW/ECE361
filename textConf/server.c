#include "utils.h"

enum account_stat {
    ACCOUNT_VALID,
    ACCOUNT_NOT_EXIST,
    WRONG_PASSWORD,
    ALREADY_ONLINE,
    NO_MORE_ACCOUNT
};

enum session_stat {
    SESSION_VALID,
    SESSION_EXISTS,
    NO_MORE_SESSION
};

const char *account_not_exist = "\nthis account does not exist!\n\n";
const char *wrong_password = "\nwrong password!\n\n";
const char *already_online = "\nthis account has alreay logged in on another machine!\n\n";
const char *no_more_account = "\ntoo many users online, need to wait for someone to logout!\n\n";


const char *session_exists = "\n this session name already exists!\n\n";
const char *no_more_session = "\n too many active sessions, need to wait for some sessions to close!\n\n";

char *online_client[MAX_ONLINE];
char *session[MAX_SESSION];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int thread_count = 0;
int online_count = 0;
int session_count = 0;

void recv_main_loop(int *listen_sockfd);
static bool process_message(struct message *msg, int sockfd);

static void do_login(struct message *msg, int sockfd);
static void do_logout(struct message *msg, int sockfd);
static void do_newsession(struct message *msg, int sockfd);

void add_account(char *client_id);
void remove_account(char *client_id);
int find_client(char *client_id);
static enum account_stat check_account(char *client_id, char *password);

int find_session(char *session_id);
void add_session(char *session_id);
static enum session_stat check_session(char *session_id);

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
        int recv_sockfd = accept_conn(listen_sockfd);
        pthread_t *new_thread = (pthread_t *)malloc(sizeof(pthread_t));
        pthread_create(new_thread, NULL, (void *)&recv_main_loop, &recv_sockfd);
    }

    return 0;
}

void recv_main_loop(int *recv_sockfd) {
    struct message *msg = (struct message *)malloc(sizeof(struct message));

    bool exit = false;
    while(!exit) {
        recv_message(msg, *recv_sockfd);
        exit = process_message(msg, *recv_sockfd);
    }
    free(msg);
    pthread_mutex_lock(&lock);
    --thread_count;
    pthread_mutex_unlock(&lock);
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
        case NEW_SESS: {
            do_newsession(msg, sockfd);
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
        case ACCOUNT_VALID: {
            msg->msg_type = LO_ACK;
            add_account(client_id);
            break;
        }
        case ACCOUNT_NOT_EXIST: {
            msg->msg_type = LO_NAK;
        memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, account_not_exist, strlen(account_not_exist));
            break;
        }
        case WRONG_PASSWORD: {
            msg->msg_type = LO_NAK;
        memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, wrong_password, strlen(wrong_password));
            break;
        }
        case ALREADY_ONLINE: {
            msg->msg_type = LO_NAK;
            memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, already_online, strlen(already_online));
            break;
        }
        case NO_MORE_ACCOUNT: {
            msg->msg_type = LO_NAK;
        memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, no_more_account, strlen(no_more_account));
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

static void do_newsession(struct message *msg, int sockfd) {
    char *session_id = (char *)msg->data;
    enum session_stat stat = check_session(session_id);
    switch(stat) {
        case SESSION_VALID: {
            msg->msg_type = NS_ACK;
            add_session(session_id);
            break;
        }
        case SESSION_EXISTS: {
            msg->msg_type = NS_NAK;
            memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, session_exists, strlen(session_exists));
            break;
        }
        case NO_MORE_SESSION: {
            msg->msg_type = NS_NAK;
        memset(msg->data, 0, MAX_DATA);
            memcpy(msg->data, no_more_session, strlen(no_more_session));
            break;
        }
        default: {
            break;
        }
    }
    send_message(msg, sockfd);
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
    if(idx == -1) {
        ereport("client to remove not found!");
    }
    online_client[idx] = "";
    //printf("\nclient %s removed\n\n", client_id);
    --online_count;
}

int find_client(char *client_id) {
    for(int i = 0; i < MAX_ONLINE; ++i) {
        char *temp_id = online_client[i];
        if(temp_id != NULL && strcmp(client_id, temp_id) == 0) {
            return i;
        }
    }
    return -1;
}

int find_session(char *session_id) {
    for(int i = 0; i < MAX_SESSION; ++i) {
        char *temp_id = session[i];
        if(temp_id != NULL && strcmp(session_id, temp_id) == 0) {
            return i;
        }
    }
    return -1;
}

void add_session(char *session_id) {
    for(int i = 0; i < MAX_SESSION; ++i) {
        if(session[i] == NULL) {
            session[i] = strdup(session_id);
            ++session_count;
            return;
        }
    }
    ereport("no more space!");
}

static enum account_stat check_account(char *client_id, char *password) {
    if(find_client(client_id) >= 0) return ALREADY_ONLINE;
        if(online_count == MAX_ONLINE) return NO_MORE_ACCOUNT;
    return ACCOUNT_VALID;
}

static enum session_stat check_session(char *session_id) {
    if(session_count == MAX_SESSION) return NO_MORE_SESSION;
    if(find_session(session_id) >= 0) return SESSION_EXISTS;
    return SESSION_VALID;
}
