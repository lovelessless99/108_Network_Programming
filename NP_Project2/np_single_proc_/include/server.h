#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define REQUEST_QUEUE_LEN 30


void server(char *port);
int create_socket (char *port);
int connect_client(int socket_fd);

char* get_IP_String(const struct sockaddr *sa);

#endif