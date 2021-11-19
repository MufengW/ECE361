#ifndef SERVER_H
#define SERVER_H
#include "utils.h"

enum account_stat {
    ACCOUNT_VALID,
    ACCOUNT_NOT_EXIST,
    WRONG_PASSWORD,
    ALREADY_LOGIN,
    NO_MORE_ACCOUNT
};

enum session_stat {
    SESSION_NOT_EXIST,
    SESSION_EXISTS,
    NO_MORE_SESSION
};

const char *account_not_exist = "\nthis account does not exist!\n\n";
const char *wrong_password = "\nwrong password!\n\n";
const char *already_login = "\nthis account has alreay logged in on another machine!\n\n";
const char *no_more_account = "\ntoo many users online, need to wait for someone to logout!\n\n";


const char *session_exists = "\nthis session name already exists!\n\n";
const char *session_not_exist = "\nthis session does not exist!\n\n";
const char *no_more_session = "\ntoo many active sessions, need to wait for some sessions to close!\n\n";
const char *already_in_a_session = "\nclient is already in a session!\n\n";
const char *client_already_in_session = "\nclient is already in this session!\n\n";
const char *not_in_session = "\nyou are not in any session!\n\n";

char *all_client[MAX_ACCOUNT];
int fd_list[MAX_ACCOUNT];
bool login_client[MAX_ACCOUNT];

char credentials[MAX_ACCOUNT][MAX_NAME];
int total_credentials = 0;

char *session[MAX_SESSION];
bool session_client_map[MAX_SESSION + 1][MAX_ACCOUNT];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int thread_count = 0;
int total_account = 0;
int session_count = 0;

static void init_global();
void recv_main_loop(int *listen_sockfd);
static void read_credentials();
static bool process_message(struct message *msg, int sockfd);

static void do_login(struct message *msg, int sockfd);
static void do_logout(struct message *msg, int sockfd);
static void do_newsession(struct message *msg, int sockfd);
static void do_joinsession(struct message *msg, int sockfd);
static void do_leavesession(struct message *msg, int sockfd);
static void do_query(struct message *msg, int sockfd);
static void do_message(struct message *msg, int sockfd);
static void do_quit(struct message *msg, int sockfd);

void add_account(char *client_id, int sockfd);
void remove_account(char *client_id);
int find_client(char *client_id);

static enum account_stat check_account(char *client_id, char *password);

int find_session(char *session_id);
void add_session(char *session_id);
void client_join_session(char *client_id, char *session_id);
static enum session_stat check_session(char *session_id);

void client_join_session(char *client_id, char *session_id);
#endif /* SERVER_H */
