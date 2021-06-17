#include "../inc/header.h"

using namespace std;

int getPort(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: ./broker [port]\n");
        exit(1);
    }
    return atoi(argv[1]);
}

void setNonBlocking(int socket)
{
    fcntl(socket, F_SETFL, O_NONBLOCK);
    return;
}

// ********************
// ** CLIENT control **
// ********************

CLIENT *newClient(char *client_id, int socket)
{
    CLIENT *client = (CLIENT *)malloc(sizeof(CLIENT));

    bzero(client->client_id, sizeof(char) * MAX_ID_LEN);
    bzero(client->password, sizeof(char) * MAX_PW_LEN);
    strncpy(client->client_id, client_id);
    client->is_online = true;
    client->is_verified = false;
    client->locate_socket = socket;
    return client;
}

CLIENT *getClient(char *client_id)
{
    string client_id_string(client_id);
    client_table_iter = client_table.find(client_id_string);
    if(client_id_iter != client_table.end())
        return client_table_iter->second;
    else
        return NULL;
}

void deleteClient(char *client_id)
{
    client_table.erase(client_id_string);
    return;
}

// **************************
// ** operations of socket **
// **************************

MESSAGE *recvData(int socket)
{
    MESSAGE *recv_message = (MESSAGE *)malloc(sizeof(MESSAGE));
    
    int recved, recved_total = 0;
    bzero(recv_message, sizeof(MESSAGE));

    while(recved_total < sizeof(MESSAGE)) {
        if((recved = recv(socket, recv_message, sizeof(MESSAGE), 0)) < 0) {
            fprintf("Error: recved from socket %d failed.\n", socket);
            return NULL;
        }
        else if(recved == 0) {
            return NULL;
        }
        recved_total += recved;
    }
    return recv_message;    
}

MESSAGE *createSendMessageHeader(CMD type, char *client_id)
{
    MESSAGE *send_message = (MESSAGE *)malloc(sizeof(MESSAGE));
    bzero(send_message, sizeof(MESSAGE));
    send_message->type = type;
    send_message->client_id = client_id;
    return send_message;
}

void sendLoginSignupResult(int socket, char *client_id, CLIENT_STAT client_stat)
{
    MESSAGE *send_message = createSendMessageHeader(CMD_SEND_CHECK_MSG, client_id);
    send_message->data.client_stat = client_stat;
    send(socket, send_message, sizeof(MESSAGE), 0);
    return;
}

void sendSubContent(int socket, QUEUE_NODE *queue_node)
{
    MESSAGE *send_message = createSendMessageHeader(CMD_SEND_CONTENT, fd_to_client[socket]->client_id);
    strncpy(send_message->data.content.topic, queue_node->topic, sizeof(char) * MAX_TOPIC_LEN);
    strncpy(send_message->data.content.summary, queue_node->summary, MAX_SUMMARY_LEN);
    send(socket, send_message, sizeof(MESSAGE), 0);
    return;
}

// ********************
// ** error handling **
// ********************



// ******************************
// ** operations of one client **
// ******************************

void clientLogin(int socket, CLIENT *client, MESSAGE *recv_message)
{
    if(strncmp(recv_message->data.password, client->password, MAX_PW_LEN) == 0) {
        if(client->is_online == true) {
            fprintf(stderr, "Socket %d: multiple login.\n", socket);
            sendLoginSignupResult(socket, client->client_id, STAT_MULTIPLE_LOGIN);
        }
        else {
            fprintf("Socket %d: successful login.");
            sendLoginSignupResult(socket, client->client_id, STAT_SUCCESSFUL_LOGIN);

            fd_to_client[socket] = client;
            client->is_online = true;
            client->locate_socket = socket;
        }
    }
    else {
        fprintf("Socket %d: wrong password.");
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
    if(keyword_string.empty()) {
        fd_to_client[socket]->subscribed[website_string].insert("*");
    }
    else {
        fd_to_client[socket]->subscribed[website_string].insert(keyword_string);
    }
    break;
}

void clientDeleteRule(int socket, string website_string, string keyword_string)
{
    if(website_string.empty()) {    // delete all rule
        fd_to_client[socket]->subscribed.clear()
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
    for(map<string, set<string> >::iterator map_iter = (fd_to_client[socket]->subscribed).begin();
        map_iter != (fd_to_client[socket]->subscribed).end(); iter_a++) {
        for(set<string>::iterator set_iter = map_iter->second.begin(); set_iter != map_iter->second.end(); set_iter++) {
            cout << "website: [" << map_iter->first << "]; keyword: [" << *set_iter << "]" << endl;
            // TODO: send pkt
        }
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

// ****************************************
// ** process control about recv_message **
// ****************************************


int childProcessing(int remote_socket, fd_set *read_original_set)
{
    MESSAGE *recv_message = recvData(remote_socket);
    if(recv_message == NULL) {
        clientClose(remote_socket, read_original_set);
        return 0;
    }

    CLIENT *client = getClient(recv_message->client_id);

    switch(recv_message->type) {
        case CMD_LOGIN_SIGNUP:
            if(client != NULL) {
                if(client->is_verified == true) {   // try to login
                    fprintf(stderr, "Socket %d: Client starts login...\n");
                    clientLogin(remote_socket, client, recv_message);
                }
                else { // check register password
                    fprintf(stderr, "Socket %d: Check register password...\n");
                    clientSignupCheck(remote_socket, client, recv_message);
                }
            }
            else {  // sign up
                fprintf(stderr, "Socket %d: Client signs up...\n");
                clientSignup(remote_socket, recv_message);
            }
            break;
        case CMD_MODIFY_RULE:
            clientModifyRule(remote_socket, recv_message);
            break;
        default:
            fprintf("Socket %d: receive other command.\n", remote_socket);
            break;
    }
    return 0;
}

void queueClear(int socket)
{
    if(fd_to_client[socket]->locate_socket == -1 || fd_to_client[socket]->is_online == false)
        return;
    while(!fd_to_client[socket]->client_queue.empty()) {
        QUEUE_NODE queue_node = fd_to_client[socket]->client_queue.front();
        
        sendSubContent(socket, &queue_node);

        fd_to_client[socket]->client_queue.pop();
    }
}