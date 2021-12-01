#include "server.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    init_global();
    signal(SIGINT, int_handler);
    if (argc != 2) {
        printf("\nusage: server <TCP port number to listen on>\n\n");
        exit(1);
    }
    char *port = argv[1];

    start_listen(port, &listen_sockfd);

    while (total_account < MAX_ACCOUNT) {
        int *recv_sockfd = malloc(sizeof(int));
        *recv_sockfd = accept_conn(listen_sockfd);
        pthread_t new_thread;
        pthread_create(&new_thread, NULL, (void *) recv_main_loop, recv_sockfd);
    }
    return 0;
}

static void int_handler() {
    printf("\ninterrupt detected, quitting...\n");
    struct message *msg = (struct message *) malloc(sizeof(struct message));
    msg->msg_type = QUIT;
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        if (fd_list[i] != -1) {
            set_str_val((char *) msg->source, all_client[i]);
            send_message(msg, fd_list[i]);
            do_quit(msg, fd_list[i]);
        }
    }
    close(listen_sockfd);
    exit(0);
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

void add_credentials(char *new_cred) {
    FILE *credentials_file = fopen("accounts", "a");
    fprintf(credentials_file, "\n%s", new_cred);
    fclose(credentials_file);
    read_credentials();
}

void recv_main_loop(int *recv_sockfd) {
    int sockfd = *recv_sockfd;
    free(recv_sockfd);
    struct message *msg = (struct message *) malloc(sizeof(struct message));

    bool end = false;
    while (!end) {
        memset(msg, 0, sizeof(*msg));
        if (!recv_message(msg, sockfd)) continue;
        if ((msg->msg_type) >= LO_ACK) ereport("unknown message type!");
        end = (msg->msg_type == EXIT || msg->msg_type == QUIT);
        (*process_message[msg->msg_type])(msg, sockfd);
        print_stat();
    }
    free(msg);
}

static void init_global() {
    read_credentials();
    int i, j;
    for (i = 0; i < MAX_SESSION; ++i) {
        session[i] = NULL;
    }
    for (j = 0; j < MAX_ACCOUNT; ++j) {
        all_client[j] = NULL;
        login_client[j] = false;
        fd_list[j] = -1;
        client_in_session[j] = -1;
        client_count[j] = 0;
    }
    for (i = 0; i < MAX_SESSION + 1; ++i) {
        for (j = 0; j < MAX_ACCOUNT; ++j) {
            session_client_map[i][j] = false;
        }
    }
}

static void do_login(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    char *password = (char *) msg->data;
    enum account_stat stat = check_account(client_id, password);
    switch (stat) {
        case ACCOUNT_VALID: {
            msg->msg_type = LO_ACK;
            add_account(client_id, sockfd);
            int client_idx = find_client(client_id);
            int session_idx = client_in_session[client_idx];
            if (session_idx != -1) set_str_val((char *) msg->data, session[session_idx]);
            else set_str_val((char *) msg->data, "");
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
    msg->msg_type = EXIT_DONE;
    send_message(msg, sockfd);
    if (close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
    int client_idx = find_client(client_id);
    login_client[client_idx] = false;
    fd_list[client_idx] = -1;
}

static void do_register(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    char *password = (char *) msg->data;

    char new_cred[MAX_NAME * 2];
    strcpy(new_cred, client_id);
    strcat(new_cred, ",");
    strcat(new_cred, password);

    enum account_stat stat = check_account(client_id, password);
    switch (stat) {
        case ACCOUNT_NOT_EXIST: {
            msg->msg_type = REG_ACK;
            set_str_val((char *) msg->data, "");
            add_credentials(new_cred);
            break;
        }
        default: {
            msg->msg_type = REG_NAK;
            set_str_val((char *) msg->data, (char *) already_registered);
            break;
        }
    }
    send_message(msg, sockfd);
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
            msg->msg_type = JN_ACK;
            bool back_to_session = session_client_map[session_idx][client_idx];
            client_join_session(client_id, session_id);
            char tmp[MAX_DATA];
            memset(tmp, 0, MAX_DATA);
            if (back_to_session) {
                sprintf(tmp, "returnning to session %s...\n", session_id);
            } else {
                sprintf(tmp, "joining session %s...\n", session_id);
            }
            set_str_val((char *) msg->data, tmp);
            break;
        }
        default: {
            break;
        }
    }
    send_message(msg, sockfd);
}

static void do_leavesession(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    int client_idx = find_client(client_id);
    int session_idx = client_in_session[client_idx];
    char tmp_msg[MAX_DATA];
    sprintf(tmp_msg, "user %s has left this session.\n", client_id);
    set_str_val((char *) msg->data, tmp_msg);
    do_message(msg, sockfd);

    bool empty = true;
    if (session_client_map[session_idx][client_idx]) {
        session_client_map[session_idx][client_idx] = false;
    }
    for (int j = 0; j < MAX_ACCOUNT; ++j) {
        if (session_client_map[session_idx][j]) {
            empty = false;
            break;
        }
    }
    if (empty && session[session_idx] != NULL) {
        session[session_idx] = NULL;
        --session_count;
    }

    client_in_session[client_idx] = -1;
    --client_count[session_idx];
    session_client_map[MAX_SESSION][client_idx] = true;
}

static void do_query(struct message *msg, int sockfd) {
    msg->msg_type = QU_ACK;
    char *tmp_session;
    char *tmp_client;
    char data[MAX_DATA];
    memset(data, 0, MAX_DATA);
    sprintf(data + strlen(data), "\nin session:");
    for (int i = 0; i < MAX_SESSION; ++i) {
        if (session[i] != NULL) {
            tmp_session = session[i];
            sprintf(data + strlen(data), "\n[%s]:\n  ", tmp_session);
            for (int j = 0; j < MAX_ACCOUNT; ++j) {
                if (session_client_map[i][j]) {
                    tmp_client = all_client[j];
                    sprintf(data + strlen(data), "%s\n  ", tmp_client);
                }
            }
        }
    }

    sprintf(data + strlen(data), "\nnot in session:\n  ");
    for (int j = 0; j < MAX_ACCOUNT; ++j) {
        if (session_client_map[MAX_SESSION][j]) {
            tmp_client = all_client[j];
            sprintf(data + strlen(data), "%s\n  ", tmp_client);
        }
    }
    set_str_val((char *) msg->data, data);
    send_message(msg, sockfd);
}

static void do_message(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    int client_idx = find_client(client_id);
    int session_idx = client_in_session[client_idx];
    msg->msg_type = MESSAGE_PRINT;

    if (session_idx == -1) {
        // not in any session
        set_str_val((char *) msg->data, (char *) not_in_session);
        send_message(msg, sockfd);
        return;
    }
    char data[MAX_DATA * 2];
    const char *msg_data = strdup((char *) msg->data);
    //printf("sending message to users in session %s...\n\n", client_session);
    // loop through session to get all client
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        if (session_client_map[session_idx][i] && client_in_session[i] == session_idx) {
            if (fd_list[i] != sockfd && fd_list[i] != -1) {
                memset(data, 0, MAX_DATA * 2);
                sprintf(data, "\n<message from %s>: %s\n", msg->source, msg_data);
                set_str_val((char *) msg->data, data);
                send_message(msg, fd_list[i]);
            }
        }
    }
}

static void do_quit(struct message *msg, int sockfd) {
    char *client_id = (char *) msg->source;
    remove_account(client_id);
    close(sockfd);
}

void add_account(char *client_id, int sockfd) {
    int client_idx = find_client(client_id);
    if (client_idx != -1) { // account already exists, which implies this account had logged out before.
        fd_list[client_idx] = sockfd;
        login_client[client_idx] = true;
        return;
    }
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
    client_in_session[client_idx] = -1;
    all_client[client_idx] = NULL;
    fd_list[client_idx] = -1;
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
    session_client_map[MAX_SESSION][client_idx] = false;
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
    printf("%s\n", client_id);
    printf("%d\n", total_credentials);
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
    int join_session_idx = find_session(session_id);
    int client_idx = find_client(client_id);
    session_client_map[join_session_idx][client_idx] = true;
    session_client_map[MAX_SESSION][client_idx] = false;
    client_in_session[client_idx] = join_session_idx;
    ++client_count[join_session_idx];
}

void print_stat() {
    printf("\n --------------- print begin ---------------\n\n");
    printf("total client count: \t%d\n\n", total_account);
    printf("total session count: \t%d\n\n", session_count);
    printf("map:\n\t");
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        printf("%s\t", (all_client[i] == NULL ? "null" : all_client[i]));
    }
    printf("\n");
    for (int i = 0; i < MAX_SESSION + 1; ++i) {
        if (i < MAX_SESSION) printf("%s\t", (session[i] == NULL ? "null" : session[i]));
        else printf("%s\t", "others");
        for (int j = 0; j < MAX_ACCOUNT; ++j) {
            printf("%d\t", session_client_map[i][j]);
        }
        printf("\n");
    }

    printf("client:\n");
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        printf("%d:%s ", i, all_client[i]);
    }
    printf("\n\n");

    printf("session: \n");
    for (int i = 0; i < MAX_SESSION; ++i) {
        printf("%d:%s ", i, session[i]);
    }
    printf("\n\n");

    printf("client_in_session:\n");
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        printf("%d:%d ", i, client_in_session[i]);
    }
    printf("\n\n");

    printf("client_count:\n");
    for (int i = 0; i < MAX_SESSION; ++i) {
        printf("%s:%d ", session[i], client_count[i]);
    }
    printf("\n\n");

    printf("fd_list:\n");
    for (int i = 0; i < MAX_ACCOUNT; ++i) {
        printf("%d:%d ", i, fd_list[i]);
    }

    printf("\n\n --------------- print done ---------------\n\n");
}
