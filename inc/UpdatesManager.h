#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <sys/wait.h>
#include <errno.h>
#include <thread>
#include <chrono>

#include "header.h"
using std::this_thread::sleep_for;

#define MAX_WEBSITE_LEN 128
#define MAX_KEYWORD_LEN 32
#define MAX_CONTENT_LEN 1024

#define MAX_TITLE_LEN 32
typedef enum {
    UPDATE_ERROR = -1,
    UPDATE_EMPTY = 0,
    UPDATE_SUCCESS = 1
} UPDATE_TYPE;


typedef struct update {
    UPDATE_TYPE type;              // -1: error, 0 or 1: normal
    time_t publish_time = 0;
    char website[MAX_WEBSITE_LEN] = "";
    char keyword[MAX_KEYWORD_LEN] = "";
    char content[MAX_CONTENT_LEN] = ""; // optional or error message
    char title[MAX_TITLE_LEN] = "";
    char cliend_id[MAX_ID_LEN] = "";
} Update;

typedef struct subscribe{
    char website[MAX_WEBSITE_LEN];
    char keyword[MAX_KEYWORD_LEN];
    bool operator==(const struct subscribe &o) const {
        return (strcmp(website, o.website)==0 && strcmp(keyword, o.keyword)==0);
    }
    bool operator<(const struct subscribe &o) const {
        return (strcmp(website, o.website)!=0 || strcmp(keyword, o.keyword)!=0);
    }
} Subscribe;

typedef struct node
{
    CLIENT* client;
    struct node* next;
}Node;

typedef struct client_list
{
    Node* head;
    Node* tail;
}Client_list;

class UpdatesManager {
private:
    time_t last_updated_time;
    int updates_count;            // count how many updates has been done
    int updates_period;           // define the period between consecutive updates
public:

    void ReadUpdates();
    // void WriteRequests();

    std::vector<Update *> updates;   
    // std::vector<Request *> requests; 
    std::map<Subscribe, Client_list>* sub_client_pair;
    UpdatesManager();
    // ~UpdatesManager();
    // void startUpdateManager(update_manager, bool END, int minute);
    void WriteRequests(std::map<string, CLIENT*> client_table);
    void GetUpdates(std::map<string, CLIENT*> client_table);       // fork python runtime, read updates from file, then enqueue to Updates
    void PushUpdates();      // push Updates to client queue
    void PushUpdate(Update *u, Client_list client_list);
    void InsertSubscription(Subscribe sub, CLIENT* client);
    void DumpUpdate(Update *u);
    void DumpUpdates();
    void DumpRequests();
};
