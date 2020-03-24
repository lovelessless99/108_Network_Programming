#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct client client;
typedef struct tube Tube;

#define WELCOME "****************************************\n** Welcome to the information server. **\n****************************************\n"
#define REQUEST_QUEUE_LEN 30


void server(char *port);
int create_socket (char *port);
int connect_client(client** user_list, int socket_fd);
void free_resource(client** user_list, Tube** tube_list,int fd);
char* get_IP_String(const struct sockaddr *sa);

#endif