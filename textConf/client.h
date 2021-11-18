#ifndef CLIENT_H
#define CLIENT_H
#include "utils.h"

bool login = false;
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
static void do_joinsession(struct message *msg);
static void do_leavesession(struct message *msg);
static void do_query(struct message *msg);
static void do_quit(struct message *msg);

#endif /* CLIENT_H */
