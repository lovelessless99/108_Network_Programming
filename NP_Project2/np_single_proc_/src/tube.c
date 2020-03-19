#include "tube.h"


Tube* create_tube(int sender, int receiver)
{
        Tube* new_tube = malloc(sizeof(Tube)); 
        new_tube->sender   = sender  ;        
        new_tube->receiver = receiver;
        new_tube->next_tube = NULL;
        pipe(new_tube->pipe_fd);
        return new_tube;
}

void push_tube(Tube** list, Tube** new_Tube)
{
        (*new_Tube)->next_tube = *list;
        *list = *new_Tube;
}

void delete_tube(Tube** list, int sender)
{
        Tube* temp = *list, *prev;
        
        // head
        if(temp && temp->sender == sender){ 
                (*list) = temp->next_tube;
                close(temp->pipe_fd[0]); close(temp->pipe_fd[1]);
                free(temp);
                temp = (*list);
        }

        while (temp != NULL) { 
                while (temp != NULL && temp->sender != sender) { 
                        prev = temp;
                        temp = temp->next_tube;
                } 
                if (temp == NULL) return; 
                prev->next_tube = temp->next_tube; 
                close(temp->pipe_fd[0]); close(temp->pipe_fd[1]);
                free(temp); 
                temp = prev->next_tube; 
        }
}


Tube* search_tube(Tube* list, int sender, int receiver)
{
        for_each_tube(list) { if( ptr->sender == sender && ptr->receiver == receiver) { return ptr;} }
        return NULL;
}

void printList(Tube* list) 
{ 
    while (list) { 
        printf("Sender: %d, Receiver: %d\n", list->sender, list->receiver);
        list = list->next_tube;
    } 
} 