#include "../inc/header.h"

using namespace std;

// **************************
// ** operations of socket **
// **************************

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

int initSocket(struct sockaddr_in *localAddr, int port)
{
    int localSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(localSocket == -1){
        fprintf(stderr, "socket() call failed!!");
        exit(1);
    }

    localAddr->sin_family = AF_INET;
    localAddr->sin_addr.s_addr = INADDR_ANY;
    localAddr->sin_port = htons(port);
    
    //setBlocking(localSocket);
    if( bind(localSocket, (struct sockaddr *)localAddr , sizeof(*localAddr)) < 0) {
        fprintf(stderr, "Can't bind() socket");
        exit(1);
    }
        
    listen(localSocket , 3);

    return localSocket;
}



// ***********************
// ** send/receive data **
// ***********************

MESSAGE *recvData(int socket)
{
    MESSAGE *recv_message = (MESSAGE *)malloc(sizeof(MESSAGE));
    
    int recved, recved_total = 0;
    bzero(recv_message, sizeof(MESSAGE));

    while(recved_total < sizeof(MESSAGE)) {
        if((recved = recv(socket, recv_message, sizeof(MESSAGE), 0)) < 0) {
            fprintf(stderr, "Error: recved from socket %d failed.\n", socket);
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
    strncpy(send_message->client_id, client_id, sizeof(char) * MAX_ID_LEN);
    return send_message;
}

void sendLoginSignupResult(int socket, char *client_id, CLIENT_STAT client_stat)
{
    MESSAGE *send_message = createSendMessageHeader(CMD_SEND_CHECK_MSG, client_id);
    send_message->data.client_stat = client_stat;
    send(socket, send_message, sizeof(MESSAGE), 0);
    return;
}

void sendSubContent(int socket, QUEUE_NODE *queue_node, bool is_last)
{
    MESSAGE *send_message = createSendMessageHeader(CMD_SEND_CONTENT, fd_to_client[socket]->client_id);
    bzero(send_message, sizeof(MESSAGE));
    if(is_last) {
        send_message->data.content.is_last = true;
    }
    else {
        send_message->data.content.is_last = false;
        strncpy(send_message->data.content.website, queue_node->website, sizeof(char) * MAX_WEBSITE_LEN);
        strncpy(send_message->data.content.topic, queue_node->topic, sizeof(char) * MAX_TOPIC_LEN);
        strncpy(send_message->data.content.summary, queue_node->summary, sizeof(char) * MAX_SUMMARY_LEN);
        send_message->data.content.publish_time = queue_node->publish_time;
    }
    send(socket, send_message, sizeof(MESSAGE), 0);
    return;
}

void processSendSubRule(int socket, MESSAGE *send_message, string website_string, string keyword_string, int *rule_num, bool is_last)
{
    if((*rule_num) % MAX_RULE_NUM == 0)
        bzero(&(send_message->data), sizeof(SUB_RULE));

    strcpy(send_message->data.sub_rule.rules[*rule_num].website, website_string.c_str());
    strcpy(send_message->data.sub_rule.rules[*rule_num].keyword, keyword_string.c_str());
    send_message->data.sub_rule.rules[*rule_num].rule_control_type = RULE_LIST;
    send_message->data.sub_rule.is_last = is_last;
    
    (*rule_num)++;
    if(is_last) {
        (*rule_num)--;
        send_message->data.sub_rule.rule_num = (*rule_num) % MAX_RULE_NUM;
        send(socket, send_message, sizeof(MESSAGE), 0);
    }
    else if(!is_last && (*rule_num) % MAX_RULE_NUM == 0) {
        send_message->data.sub_rule.rule_num = MAX_RULE_NUM;        
        send(socket, send_message, sizeof(MESSAGE), 0);
    }

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
                    fprintf(stderr, "Socket %d: Client starts login...\n", remote_socket);
                    clientLogin(remote_socket, client, recv_message);
                }
                else { // check register password
                    fprintf(stderr, "Socket %d: Check register password...\n", remote_socket);
                    clientSignupCheck(remote_socket, client, recv_message);
                }
            }
            else {  // sign up
                fprintf(stderr, "Socket %d: Client signs up...\n", remote_socket);
                clientSignup(remote_socket, recv_message);
            }
            break;
        case CMD_MODIFY_RULE:
            clientModifyRule(remote_socket, recv_message);
            break;
        default:
            fprintf(stderr, "Socket %d: receive other command.\n", remote_socket);
            break;
    }
    return 0;
}