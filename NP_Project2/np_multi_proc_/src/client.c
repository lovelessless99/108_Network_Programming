#include "client.h"


client* create_client(int pid, char *ip, char *port)
{
        client* new_client = malloc(sizeof(client));
        new_client -> env_path = strdup("bin:.");
        new_client -> name = strdup("(no name)");
        new_client -> pid = pid;
        new_client -> ip = strdup(ip);
        new_client -> port = strdup(port);
        return new_client;
}

void insert_client(client** list, client** new_client)
{
        if(*list == NULL) {
                (*new_client)->id = 1;
                *list = *new_client;
                return;
        }

        (*new_client)->id = get_available_id(*list);
        client* ptr;

        if( (*new_client)->id == 1) { 
                (*new_client)->next_client = *list; 
                *list  = *new_client;
                return;
        } 

        for(ptr = *list ; ptr->next_client && ptr->id < (*new_client)->id ; ptr = ptr->next_client); 
        (*new_client)->next_client = ptr->next_client;
        ptr->next_client = *new_client;
}

void delete_client(client** list, int pid)
{
        int count = 1;
        client* pre_client = NULL;
        for_each_client(*list)
        {      
                if(ptr->pid == pid)
                {
                        if(count == 1)
                        {
                                client* ptr_head = *list;
                                *list = (*list)->next_client;
                                free_client_memory(&ptr_head);
                                return;
                        }
                        client *next = ptr->next_client;
                        free_client_memory(&ptr);
                        pre_client->next_client = next;
                        return;
                }
                pre_client = ptr;
                count++;
        }
}


static void free_client_memory(client **node)
{
        free((*node)->env_path);
        free((*node)->name);
        free((*node)->ip);
        free((*node)->port);
        free(*node);
        *node = NULL;
}
static int get_available_id(client *list)
{

        int count = 1;
        if(list->id != 1) return 1;
        for_each_client(list)
        {
                if( ptr->next_client && ptr->next_client->id - 1 != ptr->id ) return (ptr->id + 1);
                count++;
        }
        return count;
}
