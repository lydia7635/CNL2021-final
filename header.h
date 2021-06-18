#ifndef _HEADER_CNLFINAL_H_
#define _HEADER_CNLFINAL_H_

#include <iostream>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <set>
#include <queue>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define MAX_FD          40   // Or use getdtablesize().
#define MAX_CLIENT_NUM  50
#define MAX_ID_LEN      16
#define MAX_PW_LEN      16
#define MAX_TOPIC_LEN   64
#define MAX_SUMMARY_LEN 256
#define MAX_WEBSITE_LEN 128
#define MAX_KEYWORD_LEN 32

typedef enum {
    CMD_LOGIN_SIGNUP,
    CMD_MODIFY_RULE,
    CMD_SEND_CONTENT,
    CMD_SEND_CHECK_MSG,
    CMD_NONE
} CMD;

typedef enum {
    STAT_SUCCESSFUL_LOGIN,
    STAT_SUCCESSFUL_SIGNUP,
    STAT_MULTIPLE_LOGIN,
    STAT_WRONG_PASSWORD,
    STAT_CHECK_PASSWORD,
    STAT_UNSUCCESSFUL_SIGNUP
} CLIENT_STAT;

typedef enum {
    RULE_INSERT,
    RULE_DELETE,
    RULE_LIST
} RULE_CONTROL_TYPE;

typedef struct {
    RULE_CONTROL_TYPE rule_control_type;
    char website[MAX_WEBSITE_LEN];
    char keyword[MAX_KEYWORD_LEN];
} RULE_CONTROL;

typedef struct {
    char topic[MAX_TOPIC_LEN];
    char summary[MAX_SUMMARY_LEN];
} CONTENT;

typedef struct {
    CMD type;
    char client_id[MAX_ID_LEN];
    union {
        char password[MAX_PW_LEN];
        CLIENT_STAT client_stat;
        RULE_CONTROL rule_control;
        CONTENT content;
    } data;
} MESSAGE;

typedef struct {
    char topic[MAX_TOPIC_LEN];
    char summary[MAX_SUMMARY_LEN];
    char website[MAX_WEBSITE_LEN];
} QUEUE_NODE;

typedef struct {
    char client_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
    bool is_online;
    bool is_verified;
    int locate_socket;
    map<string, set<string>> subscribed; // subscribed websites and keywords
    queue<QUEUE_NODE> client_queue;   
} CLIENT;

extern CLIENT *fd_to_client[MAX_FD];   // mapping socket fd to client pointer
extern map<string, CLIENT*> client_table;
extern map<string, CLIENT*>::iterator client_table_iter;

int getPort(int argc, char** argv);
void setNonBlocking(int socket);

int childProcessing(int remote_socket, fd_set *read_original_set);

void queueClear(int socket);

#endif