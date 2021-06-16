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

#define MAX_URL_LEN 128
#define MAX_KEYWORD_LEN 32
#define MAX_CONTENT_LEN 1024

typedef enum {
    UPDATE_ERROR = -1,
    UPDATE_EMPTY = 0,
    UPDATE_SUCCESS = 1
} UPDATE_TYPE;


typedef struct update {
    UPDATE_TYPE type;              // -1: error, 0 or 1: normal
    time_t publish_time;
    char url[MAX_URL_LEN];
    char keyword[MAX_KEYWORD_LEN];
    char content[MAX_CONTENT_LEN]; // optional or error message
} Update;

typedef struct request {
    char url[128];
    char keyword[32];
} Request;


class UpdatesManager {
private:
    time_t last_updated_time;
    int updates_count;            // count how many updates has been done
    int updates_period;           // define the period between consecutive updates
public:

    void ReadUpdates();
    void WriteRequests();

    std::vector<Update *> updates;   
    std::vector<Request *> requests; 
    UpdatesManager();
    // ~UpdatesManager();
    void AddRequest(char *url, char *keyword);
    void GetUpdates();       // fork python runtime, read updates from file, then enqueue to Updates
    void PushUpdates();      // push Updates to client queue
    void DumpUpdate(Update *u);
    void DumpUpdates();
    void DumpRequests();
};