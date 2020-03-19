#ifndef __SHELL_H
#define __SHELL_H

#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define PIPE_NUMBER 10000
#define BUFFSIZE 20000
#define CMD_COUNT 1024
#define CMD_LENGTH 10
#define SPACE " \n\t\r"
#define ENTER "\n\r"
#define PIPE_SYMBLE "|!>"

#define REMOVE_ENTER_CHAR(cmd) \
    for(int i = 0 ; i < strlen(cmd); i++) { if(strchr(ENTER, cmd[i])) { cmd[i] = '\0'; } } \

typedef struct {
    int stdin, stdout, stderr;
    bool isWait;
} Command;

typedef struct client client;

void switch_command(char *cmd);
void handler(char *cmdline);
int launch(int client_fd, client** user_list);

void who ();
void tell(char *message, int receiver_id);
void yell(char *message);
void changeName(char* new_name);
#endif