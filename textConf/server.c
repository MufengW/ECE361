#include "server.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    read_credentials();
    if (argc != 2) {
        printf("\nusage: server <TCP port number to listen on>\n\n");
        exit(1);
    }
    char *port = argv[1];

    init_global();
    int listen_sockfd;
    start_listen(port, &listen_sockfd);

    while (total_account < MAX_ACCOUNT) {
        int *recv_sockfd = malloc(sizeof(int));
        *recv_sockfd = accept_conn(listen_sockfd);

        pthread_t new_thread;
        pthread_create(&new_thread, NULL, (void *) recv_main_loop, recv_sockfd);
    }
    return 0;
}

void read_credentials() {
    FILE *credentials_file = fopen("accounts", "r");
    total_credentials = 0;
    while (!feof(credentials_file)) {
        fgets(credentials[total_credentials], MAX_NAME, credentials_file);
        total_credentials++;
    }
    fclose(credentials_file);
}

void recv_main_loop(int *recv_sockfd) {
    int sockfd = *recv_sockfd;
    free(recv_sockfd);
    struct message *msg = (struct message *) malloc(sizeof(struct message));

    bool exit = false;
    while (!exit) {
        memset(msg, 0, sizeof(*msg));
        exit = process_message(msg, sockfd);
    }
    free(msg);
    pthread_exit(0);
}

static void init_global() {
    int i, j;
    for (i = 0; i < MAX_SESSION; ++i) {
        session[i] = NULL;
    }
    for (j = 0; j < MAX_ACCOUNT; ++j) {
        all_client[j] = NULL;
        login_client[i] = false;
        fd_list[i] = -1;
    }
    for (i = 0; i < MAX_SESSION + 1; ++i) {
        for (j = 0; j < MAX_ACCOUNT; ++j) {
            session_client_map[i][j] = false;
        }
    }
}

static bool process_message(struct message *msg, int sockfd) {
    recv_message(msg, sockfd);
    switch (msg->msg_type) {
        case LOGIN: {
            do_login(msg, sockfd);
            break;
        }
        case EXIT: {
            do_logout(msg, sockfd);
            return true;
        }
        case JOIN: {
            do_joinsession(msg, sockfd);
            break;
        }
        case LEAVE_SESS: {
            do_leavesession(msg, sockfd);
            break;
        }
        case NEW_SESS: {
            do_newsession(msg, sockfd);
            break;
        }
        case QUERY: {
            do_query(msg, sockfd);
            break;
        }
        case QUIT: {
            break;
        }
        case MESSAGE: {
            do_message(msg, sockfd);
            break;
        }
        default: {
            break;
        }
    }
    return false;
}

static void do_login(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    char *password = (char *) msg->data;
    enum account_stat stat = check_account(client_id, password);
    switch (stat) {
        case ACCOUNT_VALID: {
            msg->msg_type = LO_ACK;
            add_account(client_id, sockfd);
            break;
        }
        case ACCOUNT_NOT_EXIST: {
            msg->msg_type = LO_NAK;
            set_str_val((char *) msg->data, (char *) account_not_exist);
            break;
        }
        case WRONG_PASSWORD: {
            msg->msg_type = LO_NAK;
            set_str_val((char *) msg->data, (char *) wrong_password);
            break;
        }
        case ALREADY_LOGIN: {
            msg->msg_type = LO_NAK;
            set_str_val((char *) msg->data, (char *) already_login);
            break;
        }
        case NO_MORE_ACCOUNT: {
            msg->msg_type = LO_NAK;
            set_str_val((char *) msg->data, (char *) no_more_account);
            break;
        }
        default: {
            break;
        }
    }
    send_message(msg, sockfd);
}

static void do_logout(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    if (close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
    int client_idx = find_client(client_id);
    login_client[client_idx] = false;
}

static void do_newsession(struct message *msg, int sockfd) {
    char *session_id = (char *) msg->data;
    char *client_id = (char *) msg->source;
    enum session_stat stat = check_session(session_id);
    switch (stat) {
        case SESSION_NOT_EXIST: {
            msg->msg_type = NS_ACK;
            add_session(session_id);
            client_join_session(client_id, session_id);
            break;
        }
        case SESSION_EXISTS: {
            msg->msg_type = NS_NAK;
            set_str_val((char *) msg->data, (char *) session_exists);
            break;
        }
        case NO_MORE_SESSION: {
            msg->msg_type = NS_NAK;
            set_str_val((char *) msg->data, (char *) no_more_session);
            break;
        }
        default: {
            break;
        }
    }
    // disable client from joining multiple sessions, will add this feature to lab5.
    if (!session_client_map[MAX_SESSION][find_client(client_id)]) {
        msg->msg_type = NS_NAK;
        set_str_val((char *) msg->data, (char *) already_in_a_session);
    }
    send_message(msg, sockfd);
}

static void do_joinsession(struct message *msg, int sockfd) {
    char *session_id = (char *) msg->data;
    char *client_id = (char *) msg->source;
    enum session_stat stat = check_session(session_id);
    switch (stat) {
        case SESSION_NOT_EXIST: {
            msg->msg_type = JN_NAK;
            set_str_val((char *) msg->data, (char *) session_not_exist);
            break;
        }
        case SESSION_EXISTS:
        case NO_MORE_SESSION: { // when doing join, doesn't matter whether session space is full
            int client_idx = find_client(client_id);
            int session_idx = find_session(session_id);
            if (session_client_map[session_idx][client_idx]) { // client already in session
                msg->msg_type = JN_NAK;
                set_str_val((char *) msg->data, (char *) client_already_in_session);
                break;
            }
            msg->msg_type = JN_ACK;
            client_join_session(client_id, session_id);
            break;
        }
        default: {
            break;
        }
    }
    // disable client from joining multiple sessions, will add this feature to lab5.
    if (!session_client_map[MAX_SESSION][find_client(client_id)]) {
        msg->msg_type = JN_NAK;
        set_str_val((char *) msg->data, (char *) already_in_a_session);
    }
    send_message(msg, sockfd);
}

static void do_leavesession(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    int client_idx = find_client(client_id);
    for (int i = 0; i < MAX_SESSION; ++i) {
        bool empty = true;
        if (session_client_map[i][client_idx]) {
            session_client_map[i][client_idx] = false;
        }
        for (int j = 0; j < MAX_ACCOUNT; ++j) {
            if (session_client_map[i][j]) {
                empty = false;
                break;
            }
        }
        if (empty) {
            session[i] = NULL;
        }
    }
}

static void do_query(struct message *msg, int sockfd) {
    msg->msg_type = QU_ACK;
    char *tmp_session;
    char *tmp_client;
    char data[MAX_DATA];
    memset(data, 0, MAX_DATA);
    for (int i = 0; i < MAX_SESSION; ++i) {
        if (session[i] != NULL) {
            tmp_session = session[i];
            sprintf(data + strlen(data), "\nsession %s:\n", tmp_session);
            for (int j = 0; j < MAX_ACCOUNT; ++j) {
                if (session_client_map[i][j]) {
                    tmp_client = all_client[j];
                    sprintf(data + strlen(data), "\tclient %s\n", tmp_client);
                }
            }
        }
    }
    set_str_val((char *) msg->data, data);
    send_message(msg, sockfd);
}

static void do_message(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    int client_idx = find_client(client_id);
    int session_idx = -1;
    if (session_client_map[MAX_SESSION][client_idx]) {
        // not in any session
        set_str_val((char *) msg, (char *) not_in_session);
        msg->msg_type = MESSAGE;
        send_message(msg, sockfd);
    }
    for (int i = 0; i < MAX_SESSION; ++i) {
        if (session_client_map[i][client_idx]) {
            session_idx = i;
            break;
        }
    }
    char *client_session = session[session_idx];
    printf("sending message to users in session %s...\n\n", client_session);
    // loop through session to get all client
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        if (session_client_map[session_idx][i]) {
            if (fd_list[i] != sockfd) {
                send_message(msg, fd_list[i]);
            }
        }
    }
}

void add_account(char *client_id, int sockfd) {
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        if (all_client[i] == NULL) {
            fd_list[i] = sockfd;
            all_client[i] = strdup(client_id);
            login_client[i] = true;
            session_client_map[MAX_SESSION][i] = true; // last row of the map is client that has not joined any session
            ++total_account;
            return;
        }
    }
    ereport("no more space!");
}

void remove_account(char *client_id) {
    int client_idx = find_client(client_id);
    if (client_idx == -1) {
        ereport("client to remove not found!");
    }
    all_client[client_idx] = "";
//    int session_idx = find_session(session_id);
//    session_client_map
    --total_account;
}

int find_client(char *client_id) {
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        char *temp_id = all_client[i];
        if (temp_id != NULL && strcmp(client_id, temp_id) == 0) {
            return i;
        }
    }
    return -1;
}

int find_session(char *session_id) {
    for (int i = 0; i < MAX_SESSION; ++i) {
        char *temp_id = session[i];
        if (temp_id != NULL && strcmp(session_id, temp_id) == 0) {
            return i;
        }
    }
    return -1;
}

void add_session(char *session_id) {
    for (int i = 0; i < MAX_SESSION; ++i) {
        if (session[i] == NULL) {
            session[i] = strdup(session_id);
            ++session_count;
            return;
        }
    }
    ereport("no more space!");
}

static enum account_stat check_account(char *client_id, char *password) {
    if (find_client(client_id) >= 0 && login_client[find_client(client_id)]) return ALREADY_LOGIN;
    if (total_account == MAX_ACCOUNT) return NO_MORE_ACCOUNT;
    for (int i = 0; i < total_credentials; i++) {
        char cred[MAX_NAME];
        strcpy(cred, credentials[i]);
        char *id = strtok(cred, ",");
        char *pass = strtok(NULL, "\n");
        if (strcmp(id, client_id) == 0) {
            if (strcmp(pass, password) == 0) {
                return ACCOUNT_VALID;
            } else {
                return WRONG_PASSWORD;
            }
        }
    }
    return ACCOUNT_NOT_EXIST;
}

static enum session_stat check_session(char *session_id) {
    if (find_session(session_id) == -1) return SESSION_NOT_EXIST;
    if (session_count == MAX_SESSION) return NO_MORE_SESSION;
    return SESSION_EXISTS;

}

void client_join_session(char *client_id, char *session_id) {
    int session_idx = find_session(session_id);
    int client_idx = find_client(client_id);
    session_client_map[session_idx][client_idx] = true;
    session_client_map[MAX_SESSION][client_idx] = false;
}
