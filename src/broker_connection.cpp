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

CLIENT *newClient(char *client_id)
{
    CLIENT *client = (CLIENT *)malloc(sizeof(CLIENT));

    bzero(client->client_id, sizeof(char) * MAX_ID_LEN);
    bzero(client->password, sizeof(char) * MAX_PW_LEN);
    strncpy(client->client_id, client_id);
    client->is_online = false;
    client->is_verified = false;
    client->locate_socket = -1;
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
    // create unverified client
    CLIENT *client = newClient(recv_message->client_id);
    client->is_online = true;
    client->locate_socket = socket;
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

void clientModifyRule(int socket, MESSAGE *recv_message)
{
    
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
        CMD_LOGIN_SIGNUP:
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
        CMD_MODIFY_RULE:
            clientModifyRule(remote_socket, recv_message);
            break;
        default:
            fprintf("Error: receive other command from socket %d\n", remote_socket);
            break;
    }
    return 0;

}