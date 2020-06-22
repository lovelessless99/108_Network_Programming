#ifndef _COMMANDTABLE_H
#define _COMMANDTABLE_H
#include "global.h"

typedef struct CommandTable CommandTable;

struct CommandTable{
    char*** cmdTable;
    char *line;

    int row;
    int col;
    
    int hasRedirection;
    int hasAppendRediction;
    int hasPipe;
    int hasReceive;
    int hasSender;
    
    int self_id;
    int from_id;
    int to_id;
    
    pid_t pid;

    int numOfCommands;
    int currentPosition;
    int pipetable[PROCESS_NUM][2];
    int pipeN[TOKEN_BUFFSIZE];
    int stderrTable[TOKEN_BUFFSIZE];
    

    void (*set_Command_Table)(char*, CommandTable**);
    void (*printCommandInfo)(CommandTable*);
    void (*printTable)(char***);
    void (*freeCommand)(CommandTable*);
};
int init_CommandTable(CommandTable** cmd_Table, int row, int col, int client_id, pid_t pid);
#endif