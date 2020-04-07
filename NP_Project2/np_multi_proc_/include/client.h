#ifndef _CLIENT_H
#define _CLIENT_H

#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct client {
        int id;
        int fd;
        
        char *name;
        char *env_path;

        char *ip;
        char *port;

        struct client *next_client;
} client;


#define for_each_client(list) for(client* ptr = list ; ptr ; ptr = ptr->next_client)
#define print_each_client(list) for_each_client(list) { printf("User %d: fd = %d, ip = %s, port = %s\n", ptr->id, ptr->fd, ptr->ip, ptr->port ); }

#define SEARCH_CLIENT(search_mode) \
        client* search_client_by_##search_mode (client* list, int search_mode) { \
                for_each_client(list) { if(ptr->search_mode == search_mode) { return &ptr;} } \
                return NULL; \
        }\


client* create_client(int fd, char *ip, char *port);
void insert_client(client** list, client** new_client);
void delete_client(client** list, int fd);

static int  get_available_id(client *list);
static void free_client_memory(client **node);

#endif