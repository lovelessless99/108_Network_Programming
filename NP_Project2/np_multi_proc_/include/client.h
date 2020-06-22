#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <stdlib.h>
#include <resolv.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <netdb.h>

#include "CommandTable.h"
#include "global.h"

typedef struct _client client;

//#define MAX_CLIENTS 150

struct _client
{
    char path[50];
    char name[50]; 

    char ip_address[INET_ADDRSTRLEN];
    uint16_t port;
    
    pid_t pid;
    int enable;
    
    int client_fd;
    int cnt_line;
    int id;

    CommandTable *commandTable;

};
// extern client client_info[MAX_CLIENTS];



void init_client(int clientfd, pid_t clientpid);
int get_available_client();
client *get_client(int clientfd);
client *get_client_by_id(int target_id);
client *get_client_by_pid(pid_t target_pid);
#endif
