#include "../inc/header.h"

using namespace std;

// ********************
// ** CLIENT control **
// ********************

CLIENT *newClient(char *client_id, int socket)
{
    CLIENT *client = (CLIENT *)malloc(sizeof(CLIENT));

    bzero(client->client_id, sizeof(char) * MAX_ID_LEN);
    bzero(client->password, sizeof(char) * MAX_PW_LEN);
    strncpy(client->client_id, client_id, sizeof(char) * MAX_ID_LEN);
    client->is_online = true;
    client->is_verified = false;
    client->locate_socket = socket;

    string client_id_string(client_id);
    client_table[client_id_string] = client;
    return client;
}

CLIENT *getClient(char *client_id)
{
    string client_id_string(client_id);
    client_table_iter = client_table.find(client_id_string);
    if(client_table_iter != client_table.end())
        return client_table_iter->second;
    else
        return NULL;
}

void deleteClient(char *client_id)
{
    string client_id_string(client_id);
    client_table.erase(client_id_string);
    return;
}


// ***********************************
// ** rule operations of one client **
// ***********************************

void clientLogin(int socket, CLIENT *client, MESSAGE *recv_message)
{
    if(strncmp(recv_message->data.password, client->password, MAX_PW_LEN) == 0) {
        if(client->is_online == true) {
            fprintf(stderr, "Socket %d: multiple login.\n", socket);
            sendLoginSignupResult(socket, client->client_id, STAT_MULTIPLE_LOGIN);
        }
        else {
            fprintf(stderr, "Socket %d: successful login.\n", socket);
            sendLoginSignupResult(socket, client->client_id, STAT_SUCCESSFUL_LOGIN);

            fd_to_client[socket] = client;
            client->is_online = true;
            client->locate_socket = socket;
        }
    }
    else {
        fprintf(stderr, "Socket %d: wrong password.", socket);
        sendLoginSignupResult(socket, client->client_id, STAT_WRONG_PASSWORD);
    }
    return;
}

void clientSignup(int socket, MESSAGE *recv_message)
{
    CLIENT *client = newClient(recv_message->client_id, socket);
    fd_to_client[socket] = client;
    strncpy(client->password, recv_message->data.password, MAX_PW_LEN);

    sendLoginSignupResult(socket, client->client_id, STAT_CHECK_PASSWORD);

    return;
}

void clientSignupCheck(int socket, CLIENT *client, MESSAGE *recv_message)
{
    if(client->is_online == true && socket != client->locate_socket) {
        fprintf(stderr, "Socket %d: multiple login.\n", socket);
        sendLoginSignupResult(socket, client->client_id, STAT_MULTIPLE_LOGIN);
    }
    else if(strncmp(client->password, recv_message->data.password, MAX_PW_LEN) == 0) {
        fprintf(stderr, "Socket %d: successful signup.\n", socket);
        sendLoginSignupResult(socket, client->client_id, STAT_SUCCESSFUL_SIGNUP);
        client->is_verified = true;
        client->is_online = false;
        fd_to_client[socket] = NULL;
        client->subscribed = {};
    }
    else {
        fprintf(stderr, "Socket %d: unsuccessful signup.\n", socket);
        sendLoginSignupResult(socket, client->client_id, STAT_UNSUCCESSFUL_SIGNUP);
        deleteClient(client->client_id);
        fd_to_client[socket] = NULL;
    }
    
}

void clientInsertRule(int socket, string website_string, string keyword_string)
{
    fprintf(stderr, "Socket %d: insert website [%s], keyword [%s]", socket, website_string.c_str(), keyword_string.c_str());
    website_string = returnValidUrl(website_string);
    if(website_string.empty())
        return;
    if(keyword_string.empty()) {
        fd_to_client[socket]->subscribed[website_string].clear();
    }
    else {
        fd_to_client[socket]->subscribed[website_string].insert(keyword_string);
    }

    return;
}

void clientDeleteRule(int socket, string website_string, string keyword_string)
{
    fprintf(stderr, "Socket %d: delete website [%s], keyword [%s]", socket, website_string.c_str(), keyword_string.c_str());
    if(website_string.empty()) {    // delete all rule
        fd_to_client[socket]->subscribed.clear();
    }
    else if(keyword_string.empty()) {   // delete all keywords under specific website
        fd_to_client[socket]->subscribed.erase(website_string);
    }
    else {
        if(fd_to_client[socket]->subscribed.find(website_string) != fd_to_client[socket]->subscribed.end())
            fd_to_client[socket]->subscribed[website_string].erase(keyword_string);
    }
    return;
}

void clientListRule(int socket)
{
    char client_id[MAX_ID_LEN];
    strncpy(client_id, fd_to_client[socket]->client_id, MAX_ID_LEN);
    MESSAGE *send_message = createSendMessageHeader(CMD_RETURN_RULE, client_id);
    int rule_num = 0;
    for(map<string, set<string> >::iterator map_iter = (fd_to_client[socket]->subscribed).begin();
        map_iter != (fd_to_client[socket]->subscribed).end(); map_iter++) {
        
        fprintf(stderr, "Socket %d: website [%s]\n", socket, map_iter->first.c_str());
        if(map_iter->second.empty()) {   // subscribe whole website
            processSendSubRule(socket, send_message, map_iter->first, "", &rule_num, false);
            continue;
        }

        for(set<string>::iterator set_iter = map_iter->second.begin(); set_iter != map_iter->second.end(); set_iter++) {
            cout << "website: [" << map_iter->first << "]; keyword: [" << *set_iter << "]" << endl;
            processSendSubRule(socket, send_message, map_iter->first, *set_iter, &rule_num, false);
        }
    }
    processSendSubRule(socket, send_message, "", "", &rule_num, true);

    return;
}

void queuePop(int socket)
{
    if(fd_to_client[socket]->locate_socket == -1
        || fd_to_client[socket]->is_online == false)
        return;
    while(!fd_to_client[socket]->client_queue.empty()) {
        QUEUE_NODE queue_node = fd_to_client[socket]->client_queue.front();
        
        sendSubContent(socket, &queue_node);

        fd_to_client[socket]->client_queue.pop();
    }
    return;
}

void clientModifyRule(int socket, MESSAGE *recv_message)
{
    string website_string(recv_message->data.rule_control.website);
    string keyword_string(recv_message->data.rule_control.keyword);

    map<string, set<string>>::iterator sub_iter;

    switch(recv_message->data.rule_control.rule_control_type) {
        case RULE_INSERT:
            clientInsertRule(socket, website_string, keyword_string);
            break;
        case RULE_DELETE:
            clientDeleteRule(socket, website_string, keyword_string);
            break;
        case RULE_LIST:
            clientListRule(socket);
            break;
        case QUERY_CONTENT:
            queuePop(socket);
        default:
            fprintf(stderr, "Socket %d: wrong rule_control_type.\n", socket);
            break;
    }
}

void clientClose(int socket, fd_set *read_original_set)
{
    if(fd_to_client[socket] != NULL) {
        fd_to_client[socket]->is_online = false;
        fd_to_client[socket]->locate_socket = -1;
        if(fd_to_client[socket]->is_verified == false)
            deleteClient(fd_to_client[socket]->client_id);
        fd_to_client[socket] = NULL;
    }
    fprintf(stderr, "Close client socket [%d].\n", socket);
    close(socket);
    FD_CLR(socket, read_original_set);
    return;
}