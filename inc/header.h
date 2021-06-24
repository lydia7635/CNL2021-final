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
#include <regex>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <curl/curl.h>
#include <thread>
#include <mutex>

using namespace std;

#define MAX_FD          40   // Or use getdtablesize().
#define MAX_CLIENT_NUM  50
#define MAX_RULE_NUM    10
#define MAX_ID_LEN      16
#define MAX_PW_LEN      16
#define MAX_TOPIC_LEN   64
#define MAX_SUMMARY_LEN 256
#define MAX_WEBSITE_LEN 128
#define MAX_KEYWORD_LEN 32

typedef enum {
    CMD_LOGIN_SIGNUP,
    CMD_MODIFY_RULE,
    CMD_RETURN_RULE,
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
    RULE_LIST,
    QUERY_CONTENT
} RULE_CONTROL_TYPE;

typedef struct {
    RULE_CONTROL_TYPE rule_control_type;
    char website[MAX_WEBSITE_LEN];
    char keyword[MAX_KEYWORD_LEN];
} RULE_CONTROL;

typedef struct {
    bool is_last;
    char website[MAX_WEBSITE_LEN];
    char topic[MAX_TOPIC_LEN];
    char summary[MAX_SUMMARY_LEN];
    time_t publish_time = 0;
} CONTENT;

typedef struct {
    bool is_last;
    int rule_num;
    RULE_CONTROL rules[MAX_RULE_NUM]; // one rule: website-keyword pair
} SUB_RULE;

typedef struct {
    CMD type;
    char client_id[MAX_ID_LEN];
    union {
        char password[MAX_PW_LEN];
        CLIENT_STAT client_stat;
        RULE_CONTROL rule_control;
        CONTENT content;
        SUB_RULE sub_rule;
    } data;
} MESSAGE;

typedef struct Queue_Node{
    char website[MAX_WEBSITE_LEN];
    char topic[MAX_TOPIC_LEN];
    char summary[MAX_SUMMARY_LEN];
    time_t publish_time;
    bool operator==(const struct Queue_Node &o) const {
        return (strcmp(website, o.website)==0 && strcmp(topic, o.topic)==0 && strcmp(summary, o.summary)==0);
    }
    bool operator<(const struct Queue_Node &o) const {
        return (strcmp(website, o.website)!=0 || strcmp(topic, o.topic)!=0 || strcmp(summary, o.summary)!=0);
    }
} QUEUE_NODE;

typedef struct {
    char client_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
    bool is_online;
    bool is_verified;
    int locate_socket;
    map<string, set<string>> subscribed; // subscribed websites and keywords
    set<QUEUE_NODE> client_queue;   
} CLIENT;


// ****************************
// ***** extern variables *****
// ****************************

extern CLIENT *fd_to_client[MAX_FD];   // mapping socket fd to client pointer
extern map<string, CLIENT*> client_table;
extern map<string, CLIENT*>::iterator client_table_iter;
extern mutex client_mutex;


// *********************
// ***** functions *****
// *********************

// operations of socket
int getPort(int argc, char** argv);
void setNonBlocking(int socket);
int initSocket(struct sockaddr_in *localAddr, int port);

// send/receive data
MESSAGE *recvData(int socket);
MESSAGE *createSendMessageHeader(CMD type, char *client_id);
void sendLoginSignupResult(int socket, char *client_id, CLIENT_STAT client_stat);
void sendSubContent(int socket, QUEUE_NODE *queue_node, bool is_last);
void processSendSubRule(int socket, MESSAGE *send_message, string website_string, string keyword_string, int *rule_num, bool is_last);

// error handling
string successfulReturn(string website_string);
bool availableService(string website_string);
string returnValidUrl(string website_string);

// CLIENT control
CLIENT *newClient(char *client_id, int socket);
CLIENT *getClient(char *client_id);
void deleteClient(char *client_id);

// rule operations of one client
void clientLogin(int socket, CLIENT *client, MESSAGE *recv_message);
void clientSignup(int socket, MESSAGE *recv_message);
void clientSignupCheck(int socket, CLIENT *client, MESSAGE *recv_message);
void clientInsertRule(int socket, string website_string, string keyword_string);
void clientDeleteRule(int socket, string website_string, string keyword_string);
void clientListRule(int socket);
void clientModifyRule(int socket, MESSAGE *recv_message);
void clientClose(int socket, fd_set *read_original_set);
void queuePop(int socket);

// process control about recv_message
int childProcessing(int remote_socket, fd_set *read_original_set);
void createUpdatesManagerThread(map<string, CLIENT*> client_table);


#endif