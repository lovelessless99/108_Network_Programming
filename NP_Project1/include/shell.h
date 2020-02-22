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

#define BUFFSIZE 20000
#define CMD_LENGTH 10
#define CMD_COUNT 1024
#define SPACE " \n\t\r"
typedef struct {
    int stdin, stdout, stderr;
    bool isWait;
} Command;


void readline(char** cmd);
void launch(void);
void switch_command(char *cmd);
void handler(char *cmdline);

#endif