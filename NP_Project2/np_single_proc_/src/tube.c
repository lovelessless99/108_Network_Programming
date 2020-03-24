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

void delete_all_id_tube(Tube** list, int sender_id)
{
        Tube* temp = *list, *prev;
        
        // head
        if(temp && temp->sender == sender_id){ 
                (*list) = temp->next_tube;
                close(temp->pipe_fd[0]); close(temp->pipe_fd[1]);
                free(temp);
                temp = (*list);
        }

        while (temp != NULL) { 
                while (temp != NULL && temp->sender != sender_id) { 
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

void delete_id_tube(Tube** list, int sender_id, int receiver_id)
{
        Tube* temp = *list, *prev;
        
        // head
        if(temp && temp->sender == sender_id && temp->receiver == receiver_id){ 
                (*list) = temp->next_tube;
                close(temp->pipe_fd[0]); close(temp->pipe_fd[1]);
                free(temp);
                temp = (*list);
        }

        while (temp != NULL) { 
                while (temp != NULL && (temp->sender != sender_id || temp->receiver != receiver_id)) { 
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

void printList(Tube* list) 
{ 
        while (list) { 
                printf("Sender: %d, Receiver: %d\n", list->sender, list->receiver);
                list = list->next_tube;
        } 
} 