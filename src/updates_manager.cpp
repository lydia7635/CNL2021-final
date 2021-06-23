#include "../inc/updates_manager.h"

UpdatesManager::UpdatesManager() {
    /* when UpdatesManager is constructed, first update time is current time */
    last_updated_time = time(NULL);
    std::cout << "\n[UpdateManager] Update Manager was constructed at: " << asctime(localtime(&last_updated_time)) << "\n";
}

/*********************************************************
 *  Request message type:
 *      last_request_time (in seconds)
 *      <url-1> <keyword-1> <keyword-1-1> <keyword-1-2> ...
 *      <url-2> <keyword-2> 
*********************************************************/


/*****************************
 * Expected output:
 * 357375327 
 * cliend_id 5
 * URL-1 keyword-1 keyword-2
 * URL-2 keyword-1 
 * URL-3 
 * URL-4 keyword-1 keyword-2
 * URL-5 keyword-1 keyword-2
 * ....
 *****************************/

/*****************************
 * Expected input:
 *  <start>
	    <client_id> a
	    <url> https://www.youtube.com/c/HungyiLeeNTU
	    <keyword> 機器學習
	    <content> link: https://www.youtube.com/watch?v=JXDjNh2qlfc, summary: 
 *  <end>
 *****************************/



void UpdatesManager::InsertSubscription(Subscribe sub, CLIENT* client) {
    // subscription exists
    Node new_node = {client, NULL};
    if (sub_client_pair->find(sub) != sub_client_pair->end()) {
        Client_list client_list = (*sub_client_pair)[sub];
        client_list.tail->next = &new_node;
        client_list.tail = &new_node;
    }
    else { // new subscription
        Client_list client_list = {&new_node, &new_node};
        sub_client_pair->insert(std::pair<Subscribe, Client_list>(sub, client_list));
    }
}


void UpdatesManager::WriteRequests(std::map<string, CLIENT*> client_table) {
    // sub_client_pair = new std::map<Subscribe, Client_list>;
    std::ofstream requests_file("request.txt");
    requests_file << last_updated_time << "\n";
    for (auto const& client : client_table) { 
        // might cause error, since id is char[]
        requests_file << client.second->client_id << " " << client.second->subscribed.size() << "\n";         
        for (auto const& subscription : client.second->subscribed) {
            requests_file << subscription.first << " ";           // write URL to file.
            for (auto keyword : subscription.second) {
                requests_file << keyword << " ";                  // write keywords
                Subscribe sub;
                strcpy(sub.website, subscription.first.c_str());
                strcpy(sub.keyword, keyword.c_str());
                // InsertSubscription(sub, client.second);
            }
            if (subscription.second.empty()) {  // no specific keyword
                Subscribe sub;
                strcpy(sub.website, subscription.first.c_str());
                strcpy(sub.keyword, "");
                // InsertSubscription(sub, client.second);
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

void UpdatesManager::GetUpdates(std::map<string, CLIENT*> client_table) {
    WriteRequests(client_table);
    pid_t pid = fork();
    if      (pid < 0) fprintf(stderr, "fork() error\n");
    else if (pid == 0) {
        std::cout << "[UpdateManager] python3 src/get_updates.py\n";
        if ( execlp("python3", "python3", "src/get_updates.py", (char *) 0) < 0 )
            fprintf(stderr, "execl() error, errno = %d\n", errno);
    } 
    else if (pid > 0) {
        int status;
        int ret = waitpid(pid, &status, 0);
        if (ret < 0) fprintf(stderr, "waitpid() error\n");
        else         ReadUpdates();
    }
    /** :) **/
    PushUpdates(client_table);
}

void UpdatesManager::PushUpdates(std::map<string, CLIENT*> client_table) {
    for (Update *u : updates) {
        QUEUE_NODE node;
        strncpy(node.website, u->website, MAX_WEBSITE_LEN);
        strncpy(node.summary, u->content, MAX_WEBSITE_LEN);
        strncpy(node.topic, u->title, MAX_TITLE_LEN);
        client_table[u->cliend_id]->client_queue.push(node);
        delete u;
    }
    updates.clear();
}


// void UpdatesManager::PushUpdates() {
//     Subscribe sub;
//     for (Update *u : updates) {
//         strcpy(sub.website, u->website);
//         strcpy(sub.keyword, u->keyword);
//         Client_list client_list = (*sub_client_pair)[sub];
//         PushUpdate(u, client_list);
//     }
//     // /** :) **/
//     // delete u;
//     // /** :) **/
//     // updates.clear();
// }

void UpdatesManager::PushUpdate(Update *u, Client_list client_list) {
    QUEUE_NODE new_queue_node;
    strcpy(new_queue_node.topic, u->title);
    strcpy(new_queue_node.summary, u->content);
    strcpy(new_queue_node.website, u->website);
    
    Node* current_node = client_list.head;
    while (current_node != NULL) {
        CLIENT* client = current_node->client;
        client->client_queue.push(new_queue_node);
        current_node = current_node->next;
    }
}

void UpdatesManager::DumpUpdate(Update *u) {
    if (u->type == UPDATE_ERROR) {
        std::cout << "[DumpUpdate] Type: ERROR\n";
        std::cout << "[DumpUpdate] Error Message: " << u->content << "\n"; 
    } else {
        std::cout << "[DumpUpdate] Type: SUCCESS\n";
        std::cout << "[DumpUpdate] Publish Time: " <<  asctime(localtime(&(u->publish_time))); 
        std::cout << "[DumpUpdate] URL: " << u->website << "\n"; 
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



// void UpdatesManager::DumpRequests() {
//     for (int i = 0; i < requests.size(); i++) {
//         std::cout << "\n[DumpRequest]\t " << "URL: " << requests[i]->website
//                     << "\t keyword: " << requests[i]->keyword << "\n";
//     }
// }

// void startUpdateManager(UpdatesManager update_manager, bool END, int minute) {
//     while (!END) {
//         // update_manager.GetUpdates();
//         // update_manager.DumpUpdates();
//         sleep_for(std::chrono::milliseconds(minute*60*1000));
//     }
// }


// int main(int argc, char** argv) {
//     bool END = false;
//     int minute = 15;

//     UpdatesManager update_manager;

//     // std::thread t1(startUpdateManager, update_manager, END, minute);

//     // t1.join();

//     return 0;
// }
