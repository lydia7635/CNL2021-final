#include "UpdatesManager.h"
#include "header.h"

UpdatesManager::UpdatesManager() {
    /* when UpdatesManager is constructed, first update time is current time */
    last_updated_time = time(NULL);
    #ifdef DEBUG
        std::cout << "\n[Server] Update Manager was constructed at: " << asctime(localtime(&last_updated_time)) << "\n";
    #endif
}

/**
 *  Request message type:
 *      last_request_time (in seconds)
 *      <url-1> <keyword-1> <keyword-1-1> <keyword-1-2> ...
 *      <url-2> <keyword-2> 
**/

// void UpdatesManager::WriteRequests() {
//     std::ofstream requests_file("request.txt");
//     requests_file << last_updated_time << "\n";
//     for (int i = 0; i < requests.size(); i++) {
//         requests_file << requests[i]->url << " ";
//         requests_file << requests[i]->keyword << "\n";
//     }
//     requests_file.close();
// }

// void UpdatesManager::AddRequest(char *url, char *keyword) {
//     Request *r = new Request();
//     strncpy(r->url, url, MAX_URL_LEN);
//     strncpy(r->keyword, keyword, MAX_KEYWORD_LEN);
//     requests.push_back(r);
// }

/* Expected output:
 * 357375327 
 * cliend_id 5
 * URL-1 keyword-1 keyword-2
 * URL-2 keyword-1 
 * URL-3 
 * URL-4 keyword-1 keyword-2
 * URL-5 keyword-1 keyword-2
 * ....
 */


void UpdatesManager::WriteRequests(std::map<string, CLIENT*> client_table) {
    std::ofstream requests_file("request.txt");
    requests_file << last_updated_time << "\n";
    for (auto const& client : client_table) { 
        // might cause error, since id is char[]
        requests_file << client->second->client_id << " " << client->second->subscribed.size();         
        for (auto const& subscription : client->second->subscribed) {
            requests_file << subscription->first << " ";           // write URL to file.
            for (auto keyword : subscription->second) {
                requests_file << keyword << " ";                  // write keywords
            }
            requests_file << "\n";
        }
    }
    requests_file.close();
}


void UpdatesManager::ReadUpdates() {
    std::ifstream updates_file("updates.txt");
    std::string str;
    Update *u;
    while (updates_file >> str) {
        if (str == "<start>") {
            u = new Update();
            updates.push_back(u);
            u->type = UPDATE_SUCCESS;
        } 
        else if (str == "<publish_time>") {
            updates_file >> u->publish_time;
        } 
        else if (str == "<url>") {
            updates_file >> str;
            strncpy(u->website, str.c_str(), MAX_WEBSITE_LEN);
        } 
        else if (str == "<keyword>") {
            updates_file >> str;
            strncpy(u->keyword, str.c_str(), MAX_KEYWORD_LEN);
        }
        else if (str == "<title>") {
            updates_file >> str;
            strncpy(u->title, str.c_str(), MAX_TITLE_LEN);
        } 
        else if (str == "<content>") {
            updates_file >> str;
            std::string content = "";
            while (str != "<end>") {
                content = content + str + " ";
                updates_file >> str;
            }
            strncpy(u->content, content.c_str(), MAX_CONTENT_LEN);
        }
        else if (str == "<client_id>") {
            updates_file >> str;
            strncpy(u->cliend_id, str.c_str(), MAX_ID_LEN);
        } 
        else if (str == "<error>") {
            updates_file >> str;
            u->type = UPDATE_ERROR;
            strncpy(u->content, str.c_str(), MAX_CONTENT_LEN);
        }
    }
    updates_file.close();
}

void UpdatesManager::GetUpdates() {
    WriteRequests();
    pid_t pid = fork();
    if      (pid < 0) fprintf(stderr, "fork() error\n");
    else if (pid == 0) {
        std::cout << "[Server] python3 ./hackmd_crawler.py\n";
        if ( execlp("python3", "python3", "get_updates.py", (char *) 0) < 0 )
            fprintf(stderr, "execl() error, errno = %d\n", errno);
    } 
    else if (pid > 0) {
        int status;
        int ret = waitpid(pid, &status, 0);
        if (ret < 0) fprintf(stderr, "waitpid() error\n");
        else         ReadUpdates();
    }
}

void UpdatesManager::DumpUpdate(Update *u) {
    if (u->type == UPDATE_ERROR) {
        std::cout << "[DumpUpdate] Type: ERROR\n";
        std::cout << "[DumpUpdate] Error Message: " << u->content << "\n"; 
    } else {
        std::cout << "[DumpUpdate] Type: SUCCESS\n";
        std::cout << "[DumpUpdate] Publish Time: " <<  asctime(localtime(&(u->publish_time))); 
        std::cout << "[DumpUpdate] URL: " << u->url << "\n"; 
        std::cout << "[DumpUpdate] Keyword: " << u->keyword << "\n"; 
        std::cout << "[DumpUpdate] Content: " << u->content << "\n"; 
    }
}

void UpdatesManager::DumpUpdates() {
    if (updates.size() == 0)
        std::cout << "\n[DumpUpdates] No updates.\n";
    for (int i = 0; i < updates.size(); i++)
        DumpUpdate(updates[i]);
}



void UpdatesManager::DumpRequests() {
    for (int i = 0; i < requests.size(); i++) {
        std::cout << "\n[DumpRequest]\t " << "URL: " << requests[i]->url 
                    << "\t keyword: " << requests[i]->keyword << "\n";
    }
}




int main(int argc, char** argv) {

    UpdatesManager update_manager;

    update_manager.GetUpdates();
    update_manager.DumpUpdates();

    return 0;
}