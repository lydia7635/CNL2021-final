#ifndef _HEADER_CNLFINAL_H_
#define _HEADER_CNLFINAL_H_

#include <iostream>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_FD 40   // Or use getdtablesize().
#define MAX_CLIENT_NUM 50
#define MAX_ID_LEN 16
#define MAX_PW_LEN 16

typedef enum {
    CMD_LOGIN_SIGNUP,
    CMD_MODIFY_RULE,
    CMD_SEND_CONTENT,
    CMD_SEND_CHECK_MSG,
    CMD_NONE
} CMD;

typedef struct {
    enum {
        RULE_INSERT,
        RULE_DELETE,
        RULE_LIST
    } rule_control_type;
    char website[128];
    char keyword[32];
} RULE_CONTROL;

typedef struct {
    char topic[64];
    char summary[256];
} CONTENT;

typedef struct {
    CMD type;
    char client_id[MAX_ID_LEN];
    union {
        char password[MAX_PW_LEN];
        RULE_CONTROL rule_control;
        CONTENT content;
    } data;
} MESSAGE;

typedef struct {
    char client_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
    bool is_online;
    // subscribed websites and keywords
    // dict <string, set<string>> sub;
    // client queue
} CLIENT;

int getPort(int argc, char** argv);

#endif