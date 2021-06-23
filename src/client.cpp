
#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <limits>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>


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
    char website[MAX_WEBSITE_LEN];
    char topic[MAX_TOPIC_LEN];
    char summary[MAX_SUMMARY_LEN];
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



// typedef struct {
//     char client_id[MAX_ID_LEN];
//     char password[MAX_PW_LEN]; 
// } CLIENT;


using namespace std;

int user_token;
int client_socket;
struct hostent *server_hostent;
struct sockaddr_in their_address;
string server_hostname;
vector<RULE_CONTROL> RULES;

/** ^_^ **/
char *IP;
int PORT;
// MESSAGE SEND_PACKET, RECV_PACKET;
char ID[MAX_ID_LEN];
char PASSWORD[MAX_PW_LEN]; 
/** ^_^ **/



bool isLoggedin = false;

int SelectCommand();
void Login();
void connect_to_server();
void DumpClientStatInfo(CLIENT_STAT stat);
void ListRules();
void DeleteRule();
void InsertRule();
void QueryUpdates();

int main(int argc, char* argv[]) {
    if (argc == 3) {
        
        IP  = argv[1];
        PORT = atoi(argv[2]);
        
        connect_to_server();

        cout << "Connected to server.\n";
        
        Login();

        while (1) {
            int cmd = SelectCommand();
            

            switch (cmd) {
                case 1:
                    InsertRule();
                    break;
                case 2:
                    DeleteRule();
                    break;
                case 3:
                    ListRules();
                    break;
                case 4:
                    QueryUpdates();
                    break;
                    
                case 5:
                    cout << "\nBye Bye 88!\n\n";
                    close(client_socket);
                    return 0;
                    break;
                default:
                    cout << "\nYou can only type \"1\", \"2\", \"3\", \"4\" or \"5\" QQ \n";
            }

        }   
    }
    else {
        cout << "\nUsage: ./client [BROKER IP] [BROKER PROT]\n\n";
    }   
    return 0;
}


void QueryUpdates() {
    MESSAGE SEND_PACKET, RECV_PACKET;       
    bzero(&SEND_PACKET, sizeof(SEND_PACKET));
    bzero(&RECV_PACKET, sizeof(RECV_PACKET));
    strncpy(SEND_PACKET.client_id, ID, MAX_ID_LEN); 
    SEND_PACKET.type = CMD_MODIFY_RULE;
    SEND_PACKET.data.rule_control.rule_control_type = QUERY_CONTENT;
    send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
    recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);                    
    cout << RECV_PACKET.data.content.website << "\n";
    cout << RECV_PACKET.data.content.summary << "\n";
}

void connect_to_server() {

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    
    bzero(&their_address,sizeof(their_address));

    their_address.sin_family = AF_INET; // host byte order is Little Endian
    their_address.sin_port = htons(PORT); // convert to network byte order, which is in Big Endian
    their_address.sin_addr.s_addr = inet_addr("127.0.0.1"); /*** need to pass SERVER IP ***/

    if (connect(client_socket, (struct sockaddr*) &their_address, sizeof(struct sockaddr)) == -1) {
        cout << "That host exists, but a Login Authentication service is not running.\n";
        exit(1);
    } else {
        cout << "Connected to the Login Authentication server!\n\n";
    }
}


void InsertRule () {
    MESSAGE SEND_PACKET, RECV_PACKET;       
    bzero(&SEND_PACKET, sizeof(SEND_PACKET));
    bzero(&RECV_PACKET, sizeof(RECV_PACKET));
    strncpy(SEND_PACKET.client_id, ID, MAX_ID_LEN); 
    SEND_PACKET.type = CMD_MODIFY_RULE;
    char tmp[MAX_KEYWORD_LEN];
    char confirm[64];
    SEND_PACKET.data.rule_control.rule_control_type = RULE_INSERT;
    cout << "\nPlease enter the website (URL) you want to subscribe: \n";
    cout << ">>> ";
    cin >> SEND_PACKET.data.rule_control.website;
    cout << "\nPlease enter a keyword you want to subscribe: \n";
    cout << "(Type \"N\" for no keyword subscription.)\n";
    cout << ">>> ";
    
    cin >> tmp;
    cout << "\nReconfirm your subscription (\'N\' represents not specifying keyword): \n";
    cout << "[WEBSITE] " << SEND_PACKET.data.rule_control.website << "\n";
    cout << "[KEYWORD] " << tmp << "\n";
    
    cout << "\n[Y] Send subscription [N] Discard subscription\n";
    cout << ">>> ";
    cin >> confirm;
    while ( !(confirm[0] == 'Y' || confirm[0] == 'N') || strlen(confirm) != 1) {
        cout << "\nYou can only type \"Y\" or \"N\"!\n\n";
        cout << "[Y] Send subscription [N] Discard subscription\n";
        cout << ">>> ";
        cin >> confirm;
    }
    
    if (confirm[0] == 'Y') {
        if (strncmp(tmp, "N", MAX_KEYWORD_LEN) != 0) {
            strncpy(SEND_PACKET.data.rule_control.keyword, tmp, MAX_KEYWORD_LEN);
            cout << "HAVE KEYWORD!\n" ;
        }
        cout << "\nNew subscription inserted!\n";
        send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
    }
    else if (confirm[0] == 'N') {
        cout << "Choose your command again.\n";
    }
}
void DeleteRule () {
    MESSAGE SEND_PACKET;
    bzero(&SEND_PACKET, sizeof(SEND_PACKET));
    strncpy(SEND_PACKET.client_id, ID, MAX_ID_LEN);
    SEND_PACKET.type = CMD_MODIFY_RULE;
    SEND_PACKET.data.rule_control.rule_control_type = RULE_DELETE;

    ListRules();
    int index;
    cout << "\nEnter the index of rule you want to delete: ";
    cin >> index;
    index -= 1;

    cout << "\nThe subscription rule you want to delete is: \n";
    cout << "[WEBSITE] " << RULES[index].website << "\n";
    cout << "[KEYWORD] " << RULES[index].keyword << "\n";

    char confirm[64];
    cout << "\n[Y] Delete it [N] Discard\n";
    cout << ">>> ";
    cin >> confirm;
    while ( !(confirm[0] == 'Y' || confirm[0] == 'N') || strlen(confirm) != 1) {
        cout << "\nYou can only type \"Y\" or \"N\"!\n";
        cout << "\n[Y] Delete it [N] Discard\n";
        cout << ">>> ";
        cin >> confirm;
    }
    
    if (confirm[0] == 'Y') {
        strncpy(SEND_PACKET.data.rule_control.keyword, RULES[index].keyword, MAX_KEYWORD_LEN);
        strncpy(SEND_PACKET.data.rule_control.website, RULES[index].website, MAX_WEBSITE_LEN);
        cout << "\nDelete subscription rule!\n";
        send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
    }
    else if (confirm[0] == 'N') {
        cout << "Choose your command again.\n";
    }

    send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
}

void ListRules () {
    MESSAGE SEND_PACKET, RECV_PACKET;
    RULES.clear();
    bzero(&SEND_PACKET, sizeof(SEND_PACKET));
    bzero(&RECV_PACKET, sizeof(RECV_PACKET));
    strncpy(SEND_PACKET.client_id, ID, MAX_ID_LEN); 
    SEND_PACKET.type = CMD_MODIFY_RULE;
    
    SEND_PACKET.data.rule_control.rule_control_type = RULE_LIST;
    send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
    int num = 1;
    
    cout << "\n";
    cout << "+---------+\n";
    cout << "|  RULES  |\n";
    cout << "+---------+\n";
    

    while (!RECV_PACKET.data.sub_rule.is_last) {
        recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);                    
        
        for (int i = 0; i < RECV_PACKET.data.sub_rule.rule_num; i++) {
            RULES.push_back(RECV_PACKET.data.sub_rule.rules[i]);
            cout << "[" << num++ << "]" << "\t";
            cout << RECV_PACKET.data.sub_rule.rules[i].website << "\t";
            cout << RECV_PACKET.data.sub_rule.rules[i].keyword << "\t";
            cout << "\n";
        }
    }
}

void Login () {
    MESSAGE SEND_PACKET, RECV_PACKET;
    while (!isLoggedin) {
        cout << "\nLogin or Sign up First :)\n";
        cout << "[1] Login \t[2] Sign up\n>>>> ";
        int cmd;
        cin >> cmd;
        if (cin.fail()) {
            cout << "\nYou can only type \"1\" or \"2\" >< \n";
            std::cin.clear();
            std::cin.ignore(256,'\n');
        }
        else if (cmd == 1) {
            /** Send Login Message **/
            bzero(&SEND_PACKET, sizeof(SEND_PACKET));
            SEND_PACKET.type = CMD_LOGIN_SIGNUP;
            cout << "\nPlease enter your USER  ID: ";
            cin >> SEND_PACKET.client_id;
            cout << "\nPlease enter your PASSWORD: ";
            cin >> SEND_PACKET.data.password;
            send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
            
            /** Receive Server Reply **/
            bzero(&RECV_PACKET, sizeof(RECV_PACKET));
            recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
            if (RECV_PACKET.data.client_stat == STAT_SUCCESSFUL_LOGIN) {
                cout << "\nLogin Success! :DDDDDD\n";
                isLoggedin = true;
            }
            else if (RECV_PACKET.data.client_stat == STAT_CHECK_PASSWORD) {
                cout << "\nUser ID not exists. Please try again.\n";
            }
            else {
                DumpClientStatInfo(RECV_PACKET.data.client_stat);
                cout << "\nPlease Try Again :((((\n";
            }
        }
        else if (cmd == 2) {
            /** SIGN UP **/
            bzero(&SEND_PACKET, sizeof(SEND_PACKET));
            SEND_PACKET.type = CMD_LOGIN_SIGNUP;
            cout << "\nPlease enter a new USER  ID: ";
            cin >> SEND_PACKET.client_id;
            cout << "\nPlease enter a new PASSWORD: ";
            cin >> SEND_PACKET.data.password;
            send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
            
            /** Receive Server Reply **/
            bzero(&RECV_PACKET, sizeof(RECV_PACKET));
            recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
            if (RECV_PACKET.data.client_stat == STAT_CHECK_PASSWORD) {
                cout << "\nPlease enter your PASSWORD again: ";
                string first_password(SEND_PACKET.data.password);
                cin >> SEND_PACKET.data.password;
                string second_password(SEND_PACKET.data.password);
                if (first_password == second_password) {
                    send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
                    bzero(&RECV_PACKET, sizeof(RECV_PACKET));
                    recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
                    DumpClientStatInfo(RECV_PACKET.data.client_stat);

                    /* Automatic Login */
                    send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
                    bzero(&RECV_PACKET, sizeof(RECV_PACKET));
                    recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
                    DumpClientStatInfo(RECV_PACKET.data.client_stat);
                    
                    isLoggedin = true;
                    break;
                }
                
                else {
                    for (int i = 0; i < 3; i++) {
                        if (first_password != second_password) {
                            cout << "\nPASSWORDs did not match. Please try again.\n";
                            cout << "\nPlease enter your PASSWORD again: ";
                            cin >> SEND_PACKET.data.password;
                            string second_password(SEND_PACKET.data.password);
                        }
                        else {
                            send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
                            bzero(&RECV_PACKET, sizeof(RECV_PACKET));
                            recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
                            DumpClientStatInfo(RECV_PACKET.data.client_stat);
                            
                            /* Automatic Login */
                            send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
                            bzero(&RECV_PACKET, sizeof(RECV_PACKET));
                            recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
                            DumpClientStatInfo(RECV_PACKET.data.client_stat);
                            
                            isLoggedin = true;
                            break;
                        }
                    }
                    if (!isLoggedin)
                        cout << "\nGuess you forgot what you've typed for sign up PASSWORD... ;))))\n";
                }

            }
            else if (RECV_PACKET.data.client_stat == STAT_SUCCESSFUL_SIGNUP) {
                /* Automatic Login */
                send(client_socket, &SEND_PACKET, sizeof(SEND_PACKET), MSG_WAITALL);
                bzero(&RECV_PACKET, sizeof(RECV_PACKET));
                recv(client_socket, &RECV_PACKET, sizeof(RECV_PACKET), MSG_WAITALL);
                DumpClientStatInfo(RECV_PACKET.data.client_stat);
                
                isLoggedin = true;
                break;
            }
            else {
                DumpClientStatInfo(RECV_PACKET.data.client_stat);
                cout << "\nPlease Try Again :((((\n";
            }

        }
        else   cout << "\nYou can only type \"1\" or \"2\" >< \n";
    }
    if (isLoggedin) {
        strncpy(ID, SEND_PACKET.client_id, MAX_ID_LEN);
        cout << "\nWelcome " << ID << " :)))))\n";
    }
}


int SelectCommand() {
    while (1) {
        cout << "\n";
        cout << "+------------------------+\n";
        cout << "| [1] INSERT A RULE      |\n";
        cout << "| [2] DELETE A RULE      |\n";
        cout << "| [3] LIST INSERTED RULE |\n";
        cout << "| [4] QUERY CONTENT      |\n";
        cout << "| [5] EXIT               |\n";
        cout << "+------------------------+\n";
        cout << "\nType your command: " ;
        int cmd;
        cin >> cmd;
        if (cin.fail()) {
            cout << "\nYou can only type \"1\", \"2\", \"3\", \"4\" or \"5\" QQ \n";
            std::cin.clear();
            std::cin.ignore(256,'\n');
        }
        else if (cmd > 5 || cmd < 1) {
            cout << "\nYou can only type \"1\", \"2\", \"3\", \"4\" or \"5\" QQ \n";
        }
        else 
            return cmd;
    }
}

void DumpClientStatInfo (CLIENT_STAT stat) {
    cout << "\n";
    switch (stat) {
        case STAT_SUCCESSFUL_LOGIN:
            cout << "SUCCESSFUL LOGIN!\n";
            break;
        case STAT_SUCCESSFUL_SIGNUP:
            cout << "SUCCESSFUL SIGNUP!\n";
            break;
        case STAT_MULTIPLE_LOGIN:
            cout << "MULTIPLE LOGIN!\n";
            break;
        case STAT_WRONG_PASSWORD:
            cout << "WRONG PASSWORD!\n";
            break;
        case STAT_CHECK_PASSWORD:
            cout << "CHECK PASSWORD!\n";
            break;
        case STAT_UNSUCCESSFUL_SIGNUP:
            cout << "UNSUCCESSFUL SIGNUP!\n";
            break;
        default:
            cout << "WHAT?\n";
    }
}
