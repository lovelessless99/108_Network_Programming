#ifndef _TUBE_H
#define _TUBE_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct tube {
        int sender;
        int receiver;
        int pipe_fd[2];
        struct tube *next_tube;
} Tube;

#define for_each_tube(list) for(Tube* ptr = list ; ptr ; ptr = ptr->next_tube)

Tube* create_tube(int sender, int receiver);
void push_tube(Tube** list, Tube** new_Tube);
void delete_tube(Tube** list, int sender);
Tube* search_tube(Tube* list, int sender, int receiver);
void printList(Tube* list);

#endif 