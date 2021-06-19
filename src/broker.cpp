#include "../inc/header.h"

using namespace std;

CLIENT *fd_to_client[MAX_FD];   // mapping socket fd to client pointer
map<string, CLIENT*> client_table;
map<string, CLIENT*>::iterator client_table_iter;

void startUpdateManager(UpdatesManager update_manager, int minute) {
    while (1) {
        update_manager.GetUpdates(client_table);
        // update_manager.DumpUpdates();
        sleep_for(std::chrono::milliseconds(minute*60*1000));
    }
}

int main(int argc, char **argv)
{
    //*************************** Servers, CLIENTs, and Queue Preprocessing ***************************//

    // preparing structure for all clients
    for(int i = 0; i < MAX_FD; ++i)
        fd_to_client[i] = NULL;
    
    client_table.clear();

    int minute = 15;

    UpdatesManager update_manager;
    std::thread t1(startUpdateManager, update_manager, minute);

    //*************************** Socket Preprocessing ***************************//
    // preparing socket
    int local_socket, remote_socket;
    int port = getPort(argc, argv);

    struct sockaddr_in local_addr, remote_addr;
    int addr_len = sizeof(struct sockaddr_in);
    local_socket = initSocket(&local_addr, port);

    // preparing select() table
    fd_set read_original_set, read_working_set;

    FD_ZERO(&read_original_set);
    FD_SET(local_socket, &read_original_set);

    setNonBlocking(local_socket);

    fprintf(stderr, "The broker is waiting for connections...\n");
    fprintf(stderr, "Broker port: %d\n", port);


    //*************************** Connect to clients ***************************//
    while (1) {
        read_working_set = read_original_set;
        if (select(MAX_FD, &read_working_set, NULL, NULL, NULL) == -1)
            continue;

        // check whether there is a new connection
        if(FD_ISSET(local_socket, &read_working_set)) {
            remote_socket = accept(local_socket, (struct sockaddr *)&remote_addr, (socklen_t*)&addr_len);
            if (remote_socket < 0) {
                fprintf(stderr, "accept failed!");
                exit(1);
            }
            FD_SET(remote_socket, &read_original_set);

            fprintf(stderr, "Accept: new connection on socket [%d]\n", remote_socket);
            continue;
        }

        remote_socket = -1;
        for(int i = 3; i < MAX_FD; ++i) {
            if(FD_ISSET(i, &read_working_set) && i != local_socket) { // != target fd
                remote_socket = i;
                fprintf(stderr, "Received some data from socket [%d] ...\n", remote_socket);
                int child_processing_error = childProcessing(remote_socket, &read_original_set);
                if(child_processing_error)
                    fprintf(stderr, "Error %d: child processing in socket fd: %d\n", child_processing_error, remote_socket);
            }
        }
    }
    t1.join();
    return 0;
}