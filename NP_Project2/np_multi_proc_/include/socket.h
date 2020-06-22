#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <stdlib.h>
#include <resolv.h>
#include <unistd.h>

#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>

#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <dirent.h>

#include "CommandTable.h"



typedef struct addrinfo Addrinfo;
typedef struct _client client;
typedef enum option Option;

enum option {LOGIN, LOGOUT, YELL, CHANGENAME, RECEIVEPIPE, SENDPIPE};

void server(char* port);
void create_socket(int *sockfd, char* port);
void broadcast(pid_t client_pid, Option option, char *_message, int from_id, int to_id);
int connect_client(int sockfd);
void reset_client(pid_t);

//void createFIFO();
int append(char **str, const char *buf, int size);
void clear_tmp_directory(const char* dir_name);
void register_signal();
void recv_broadcast();
void sigusr1_handler();

#endif
