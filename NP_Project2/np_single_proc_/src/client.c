#include "client.h"

client* create_client(int fd, char *ip, char *port)
{
        client* new_client = malloc(sizeof(client*));
        new_client -> env_path = strdup("bin:.");
        new_client -> name = strdup("(no name)");
        new_client -> fd = fd;
        new_client -> ip = strdup(ip);
        new_client -> port = strdup(port);
        return new_client;
}

void insert_client(client** list, client** new_client)
{
        (*new_client)->id = get_available_id(*list);

        client* ptr;
        for(ptr = *list ; ptr->id < (*new_client)->id ; ptr = ptr->next_client); 
        (*new_client)->next_client = ptr->next_client;
        ptr->next_client = *new_client;
}

void delete_client(client** list, int fd)
{
        for_each_client(*list)
        {      
                if(ptr->next_client->fd == fd)
                {
                        ptr->next_client = ptr->next_client->next_client;
                        free_client_memory( &(ptr->next_client) );
                }
        }
}

// client* search_client_by_fd(client* list, int fd)
// {
//         for_each_client(list) { if(ptr->fd == fd) { return ptr;} } 
// }

// client* search_client_by_id(client* list, int id)
// {
//         for_each_client(list) { if(ptr->fd == id) { return ptr;} } 
// }



static void free_client_memory(client **node)
{
        free((*node)->env_path);
        free((*node)->name);
        free((*node)->ip);
        free((*node)->port);
        free(*node);
}
static int get_available_id(client *list)
{
        int count = 1;
        for_each_client(list)
        {
                if( ptr->next_client->id - 1 != ptr->id ) return (ptr->id + 1);
                count++;
        }
        return count;
}