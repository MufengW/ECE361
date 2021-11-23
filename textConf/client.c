#include "client.h"

int main() {
    pthread_t prompt_thread;
    pthread_create(&prompt_thread, NULL, (void *) get_prompt, NULL);


    pthread_t message_thread;
    pthread_create(&message_thread, NULL, (void *) get_message, NULL);
    
    pthread_join(prompt_thread, 0);
    return 0;
}

void get_message() {
    struct message *msg = (struct message *) malloc(sizeof(struct message));
    while (1) {
        if (connected) {
            recv_message(msg, sockfd);
            process_incoming_msg(msg);
        }
    }
}

void process_incoming_msg(struct message *msg) {
    if(!connected) return;
    switch (msg->msg_type) {
        case LO_ACK:
        case LO_NAK:
            process_login(msg);
            break;
        case NS_ACK:
        case NS_NAK:
            process_newsession(msg);
            break;
        case QU_ACK:
            process_query(msg);
            break;
        case JN_ACK:
        case JN_NAK:
            process_joinsession(msg);
            break;
        case MESSAGE:
            process_message(msg);
            break;
        default:
            break;
    }
}

void get_prompt() {
    bool exit = false;
    struct message *msg = (struct message *) malloc(sizeof(struct message));
    while (!exit) {
        exit = get_and_process_prompt(msg);
    }
    free(msg);
}

bool get_and_process_prompt(struct message *msg) {
    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);
    get_input(buf);
    return process_input(msg, buf);
}

void get_input(char *buf) {
    if (!login) {
        printf("\nInput your prompt below:\n\n>> ");
    } else {
        printf("\n\n%s:$ ", current_client);
    }
    fgets(buf, MAX_DATA, stdin);
}

static bool process_input(struct message *msg, char *buf) {
    char *data = strdup(buf); // make a copy of original data to avoid overwrite
    if (strlen(data) == 0) {
        printf("\ndata empty, try input again!\n\n");
        return false;
    }
    char delim[] = " \n\t\v\f\r";
    char *first_word = strtok(data, delim);
    if (first_word == NULL) {
        printf("\ndata empty, try input again!\n\n");
        return false;
    }
    set_str_val((char *)msg->data, buf);
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
            do_joinsession(msg);
            break;
        }
        case LEAVE_SESS: {
            do_leavesession(msg);
            break;
        }
        case NEW_SESS: {
            do_newsession(msg);
            break;
        }
        case QUERY: {
            do_query(msg);
            break;
        }
        case QUIT: {
            do_quit(msg);
            return true;
        }
        case MESSAGE: {
            do_message(msg);
            break;
        }
        case AGAIN: {
            break;
        }
        default: {
            break;
        }
    }
    return false;
}


enum type get_type(char *first_word) {
    if (strcmp(first_word, "/login") == 0) return LOGIN;
    if (strcmp(first_word, "/logout") == 0) return EXIT;
    if (strcmp(first_word, "/joinsession") == 0) return JOIN;
    if (strcmp(first_word, "/leavesession") == 0) return LEAVE_SESS;
    if (strcmp(first_word, "/createsession") == 0) return NEW_SESS;
    if (strcmp(first_word, "/list") == 0) return QUERY;
    if (strcmp(first_word, "/quit") == 0) return QUIT;

    if(first_word[0] == '/') {
        printf("unrecognized command, do you want to send it as a message?(y/n)\n\n>> ");
        char buf[MAX_DATA];
        memset(buf, 0, MAX_DATA);
        fgets(buf, MAX_DATA, stdin);
        if((strcmp(buf,"y\n") == 0) || (strcmp(buf, "yes\n") == 0)) {
            return MESSAGE;
        } else {
            printf("ignoring...\n\n");
            return AGAIN;
        }
    }
    return MESSAGE;
}

void connect_to_server(char *server_ip, char *server_port, int *sockfd) {
    struct addrinfo hints, *servinfo;
    char s[INET_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(server_ip, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    if ((*sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("client: socket");
        exit(1);
    }
    if (connect(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(*sockfd);
        perror("client: connect");
        exit(1);
    }
    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *) servinfo->ai_addr), s, sizeof(s));
    freeaddrinfo(servinfo);
}

static void do_login(struct message *msg) {
    if (login) {
        printf("\nyou have already logged in to %s on this machine!\n\n", current_client);
        return;
    }
    pthread_mutex_lock(&lock_login);
    done_login = false;
    pthread_mutex_unlock(&lock_login);

    char delim[] = " \n\t\v\f\r";
    char *client_id = strtok(NULL, delim);
    char *password = strtok(NULL, delim);
    char *server_ip = strtok(NULL, delim);
    char *server_port = strtok(NULL, delim);
    if (server_port == NULL) {
        printf("\nlogin format error, please login with command:\n\n/login <client id> <password> <server-ip> <server-port>\n\n");
        return;
    }

    detect_extra_input();

    if (!connected) {
        connect_to_server(server_ip, server_port, &sockfd);
        connected = true;
    }

    set_str_val((char *) msg->data, password);
    msg->size = strlen((const char *) msg->data);
    set_str_val((char *) msg->source, client_id);
    send_message(msg, sockfd);
    // need to wait for process ack
    while(!done_login) {
        pthread_yield();
    }
}

static void process_login(struct message *msg) {
    switch (msg->msg_type) {
        case LO_ACK: {
            printf("\nlogin successful!\n\n");
            login = true;

            current_client = (char *) malloc(sizeof(char) * MAX_NAME);
            set_str_val(current_client, (char *) msg->source);
            break;
        }
        case LO_NAK: {
            printf("%s", msg->data);
            login = false;
            connected = false;
            close(sockfd);
            break;
        }
        default: {
            ereport("unkown message type!");
            break;
        }
    }

    // login command finished
    pthread_mutex_lock(&lock_login);
    done_login = true;
    pthread_mutex_unlock(&lock_login);
}

static void do_logout(struct message *msg) {
    if (!login) {
        printf("\nyou have not logged in to any account\n\n");
        return;
    }
    detect_extra_input();

    set_str_val((char *) msg->source, current_client);
    send_message(msg, sockfd);

    pthread_mutex_lock(&lock);
    login = false;
    connected = false;
    pthread_mutex_unlock(&lock);
    if (close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
    printf("\naccount %s have successfully logged out!\n\n", current_client);
    current_client = "";
}

static void do_newsession(struct message *msg) {
    if (!login) {
        printf("\nyou need to login first!\n\n");
        return;
    }

    pthread_mutex_lock(&lock_newsession);
    done_newsession = false;
    pthread_mutex_unlock(&lock_newsession);

    char delim[] = " \n\t\v\f\r";
    char *session_id = strtok(NULL, delim);

    if (session_id == NULL) {
        printf("\ncreate new session format error, please use: \n\n/createsession <session ID>\n\n");
        return;
    }

    detect_extra_input();

    set_str_val((char *) msg->source, current_client);

    set_str_val((char *) msg->data, session_id);

    send_message(msg, sockfd);

    // need to wait for process ack
    while(!done_newsession) {
        pthread_yield();
    }
}

static void process_newsession(struct message *msg) {
    switch (msg->msg_type) {
        case NS_ACK: {
            char *session_id = (char *) msg->data;
            printf("\nnew session %s created!\n\n", session_id);
            current_session = session_id;
            break;
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

    // newsession command finished
    pthread_mutex_lock(&lock_newsession);
    done_newsession = true;
    pthread_mutex_unlock(&lock_newsession);
}

static void do_joinsession(struct message *msg) {
    if (!login) {
        printf("\nyou need to login first!\n\n");
        return;
    }

    pthread_mutex_lock(&lock_joinsession);
    done_joinsession = false;
    pthread_mutex_unlock(&lock_joinsession);

    char delim[] = " \n\t\v\f\r";
    char *session_id = strtok(NULL, delim);
    if (session_id == NULL) {
        printf("\njoin session format error, please use: \n\n/joinsession <session ID>\n\n");
        return;
    }

    detect_extra_input();

    set_str_val((char *) msg->source, current_client);

    set_str_val((char *) msg->data, session_id);

    send_message(msg, sockfd);

    // need to wait for process ack
    while(!done_joinsession) {
        pthread_yield();
    }

}

static void process_joinsession(struct message *msg) {
    switch (msg->msg_type) {
        case JN_ACK: {
            char *session_id = (char *) msg->data;
            printf("\njoining session %s...\n\n", session_id);
            current_session = session_id;
            break;
        }
        case JN_NAK: {
            printf("%s", msg->data);
            break;
        }
        default: {
            ereport("unkown message type!");
            break;
        }
    }

    // joinsession command finished
    pthread_mutex_lock(&lock_joinsession);
    done_joinsession = true;
    pthread_mutex_unlock(&lock_joinsession);
}

static void do_leavesession(struct message *msg) {
    if (!login) {
        printf("\nyou need to login first!\n\n");
        return;
    }
    detect_extra_input();

    set_str_val((char *) msg->source, current_client);
    send_message(msg, sockfd);
    printf("\nclient %s has left all the sessions!\n\n", current_client);
}

static void do_query(struct message *msg) {
    if (!login) {
        printf("\nyou need to login first!\n\n");
        return;
    }

    pthread_mutex_lock(&lock_query);
    done_query = false;
    pthread_mutex_unlock(&lock_query);

    detect_extra_input();

    send_message(msg, sockfd);

    while(!done_query) {
        pthread_yield();
    }
}

static void process_query(struct message *msg) {
    if (msg->msg_type == QU_ACK) {
        printf("%s", msg->data);
    } else {
        ereport("unkown message type!");
    }
    pthread_mutex_lock(&lock_query);
    done_query = true;
    pthread_mutex_unlock(&lock_query);
}

static void do_message(struct message *msg) {
    if (!login) {
        printf("\nyou need to login first!\n\n");
        return;
    }

    set_str_val((char *)msg->source, current_client);
    send_message(msg, sockfd);
    printf("\nyour message has been sent\n\n");
}

static void process_message(struct message *msg) {
    printf("%s%s:$ ", msg->data, current_client);
    fflush(stdout);
}

static void do_quit(struct message *msg) {
    if (!login) {
        printf("\ngoodbye!\n\n");
        return;
    }
    detect_extra_input();

    set_str_val((char *)msg->source, current_client);
    send_message(msg, sockfd);
    printf("\ngoodbye!\n\n");
    return;
}

void detect_extra_input() {
    char delim[] = " \n\t\v\f\r";
    char *extra_input = strtok(NULL, delim);
    if (extra_input) {
        printf("\nextra input dected and ignored...\n\n");
    }
}
