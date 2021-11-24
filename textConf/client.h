#ifndef CLIENT_H
#define CLIENT_H
#include "utils.h"

pthread_t prompt_thread;
pthread_t message_thread;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lock_login = PTHREAD_MUTEX_INITIALIZER;
bool done_login;

pthread_mutex_t lock_logout = PTHREAD_MUTEX_INITIALIZER;
bool done_logout;

pthread_mutex_t lock_newsession = PTHREAD_MUTEX_INITIALIZER;
bool done_newsession;

pthread_mutex_t lock_joinsession = PTHREAD_MUTEX_INITIALIZER;
bool done_joinsession;

pthread_mutex_t lock_query = PTHREAD_MUTEX_INITIALIZER;
bool done_query;

bool connected = false;
bool login = false;
char *current_client = "";
char *current_session = "";
int sockfd;

static void int_handler();

void get_message();
void get_prompt();
void get_input(char *buf);
void detect_extra_input();
static bool process_input(struct message *msg, char *buf);
enum type get_type(char *first_word);
bool get_and_process_prompt(struct message *msg);
void connect_to_server(char *server_ip, char *server_port, int *sockfd);

static void process_login(struct message *msg);
static void process_logout(struct message *msg);
static void process_newsession(struct message *msg);
static void process_joinsession(struct message *msg);
static void process_query(struct message *msg);
static void process_message(struct message *msg);

static void do_login(struct message *msg);
static void do_logout(struct message *msg);
static void do_newsession(struct message *msg);
static void do_joinsession(struct message *msg);
static void do_leavesession(struct message *msg);
static void do_query(struct message *msg);
static void do_quit(struct message *msg);
static void do_message(struct message *msg);
static void do_again(struct message *msg);

static void (*do_input[20])(struct message *msg) = {
    do_login, // LOGIN
    do_logout, // EXIT
    do_newsession, //NEW_SESS
    do_joinsession, //JOIN
    do_leavesession, //LEAVE_SESS
    do_query, //QUERY
    do_quit, //QUIT
    do_message, //MESSAGE
    do_again, //AGAIN

    process_login, // LO_ACK
    process_login, //LO_NAK
    process_logout, //LOGOUT_DONE
    process_newsession, // NS_ACK
    process_newsession, // NS_NAK
    process_joinsession, //JN_ACK
    process_joinsession, //JN_NAK
    process_query, //QU_ACK
    process_message //MESSAGE_PRINT
};

#endif /* CLIENT_H */
