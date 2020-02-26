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
#define PIPE_SYMBLE "|!>"
typedef struct {
    int stdin, stdout, stderr;
    bool isWait;
} Command;


void switch_command(char *cmd);
void readline(char** cmd);
void handler(char *cmdline);
void launch(void);

#endif