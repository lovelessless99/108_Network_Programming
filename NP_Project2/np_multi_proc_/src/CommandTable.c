#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "CommandTable.h"

int init_CommandTable(CommandTable** cmd_Table, int row, int col, int client_id, pid_t client_pid);
static void set_Command_Table(char* line, CommandTable** cmdTable);
static void printCommandInfo(CommandTable* cmdTable);
static void printTable(char*** pipetable);
static void freeCommand(CommandTable* cmdTable);
static int tokenizer(char *line, char* delim, char** destination, int size);


/*
    name: init_CommandTable
    desription: initialize struct CommandTable
*/
int init_CommandTable(CommandTable** cmd_Table, int row, int col, int client_id, pid_t client_pid)
{
    if( (*cmd_Table = malloc(sizeof(CommandTable))) == NULL) return -1;

    (*cmd_Table)->row = row;
    (*cmd_Table)->col = col;
    
    (*cmd_Table)->pid = client_pid;

    (*cmd_Table)->self_id = client_id;
    (*cmd_Table)->from_id = client_id;
    (*cmd_Table)->to_id = client_id;
    
    (*cmd_Table)->hasPipe = 0;
    (*cmd_Table)->hasRedirection = 0;
    (*cmd_Table)->hasAppendRediction = 0;
    (*cmd_Table)->numOfCommands = 0;
    (*cmd_Table)->currentPosition = 0;
    
    (*cmd_Table)->cmdTable = malloc(sizeof(char**) * row);
    for(int i = 0 ; i < row; i++)
    {
        (*cmd_Table)->cmdTable[i] = malloc(sizeof(char*) * CMD_PARAM); 
        if(!(*cmd_Table)->cmdTable[i])
            fprintf(stderr, "pipetable memory allocation error");
    }

    for(int i = 0 ; i <= PROCESS_NUM ; i++){
        (*cmd_Table)->pipetable[i][0] = STDIN_FILENO;
        (*cmd_Table)->pipetable[i][1] = STDOUT_FILENO;
    }

    for(int i = 0 ; i <= TOKEN_BUFFSIZE ; i++){
        (*cmd_Table)->pipeN[i] = 0;
        (*cmd_Table)->stderrTable[i] = 0;
    }

    (*cmd_Table)->set_Command_Table = set_Command_Table;
    (*cmd_Table)->printCommandInfo = printCommandInfo;
    (*cmd_Table)->printTable = printTable;
    (*cmd_Table)->freeCommand = freeCommand;

    return 0;
}



/*
    name: set_cmd_table
    desription: split the input line and save to command table -> cmdtable
*/
static void set_Command_Table(char* line,  CommandTable** cmdTable)
{
    (*cmdTable)->from_id = (*cmdTable)->self_id;
    (*cmdTable)->to_id = (*cmdTable)->self_id;

    (*cmdTable)->hasPipe = strchr(line, '|') != NULL;
    (*cmdTable)->line = strdup(line);
    (*cmdTable)->hasRedirection = 0;
    (*cmdTable)->hasAppendRediction = 0;
    (*cmdTable)->hasReceive = 0;
    (*cmdTable)->hasSender = 0;

    (*cmdTable)->numOfCommands = 0;
  
    char tmp[20000];
    strcpy(tmp, line);
    int buffersize = (*cmdTable)->row;
    char **cmd_tokenizer = malloc(sizeof(char*) * buffersize * 2 );
    int pos = tokenizer(tmp, " \n\t\a", cmd_tokenizer, buffersize * 2);
    int numOfCommands = 0;
    int command_pointer = 0;

    for(int i = 0 ; i < pos; i++)
    {
        int cur = ((*cmdTable)->currentPosition + numOfCommands);
        if(strchr(cmd_tokenizer[i], '|') != NULL)
        {
            char str[10];
            strncpy(str, cmd_tokenizer[i]+1, strlen(cmd_tokenizer[i]));
            if(strlen(cmd_tokenizer[i]) == 1) (*cmdTable)->pipeN[cur] = 1;
            else (*cmdTable)->pipeN[cur] = atoi(str);

            if( i != pos-1 ){
                numOfCommands++;
                command_pointer = 0;
            }
            continue;
        }

        else if(!strcmp(cmd_tokenizer[i], ">") || !strcmp(cmd_tokenizer[i], ">>"))
        {
            
            if(!strcmp(cmd_tokenizer[i], ">" ))  (*cmdTable)->hasRedirection     = 1;
            if(!strcmp(cmd_tokenizer[i], ">>"))  (*cmdTable)->hasAppendRediction = 1;
            (*cmdTable)->pipeN[cur] = 1;
            numOfCommands++;
            command_pointer = 0;
            continue;
        }

        else if (strchr(cmd_tokenizer[i], '>') && strlen(cmd_tokenizer[i]) > 1)
        {
            sscanf(cmd_tokenizer[i], ">%d", &(*cmdTable)->to_id);
            (*cmdTable)->hasSender = 1;

            continue;
        }

        else if (strchr(cmd_tokenizer[i], '<') && strlen(cmd_tokenizer[i]) > 1)
        {   
            sscanf(cmd_tokenizer[i], "<%d", &(*cmdTable)->from_id);
            (*cmdTable)->hasReceive = 1;
            continue;
        }

        else{
            (*cmdTable)->cmdTable[cur][command_pointer++] = strdup(cmd_tokenizer[i]);
        }
    }
    
    free(cmd_tokenizer);

    (*cmdTable)->numOfCommands = numOfCommands + 1;
}



/*
    name: printCommandInfo
    desription: print Command Infomation without matrix contents
*/
static void printCommandInfo(CommandTable* cmdTable)
{
//   if(!cmdTable) return;
//     printf("\n============ Commands Info. ============\n");
//     printf("Size: %d x %d \n", cmdTable->row, cmdTable->col);
//     printf("# of Commands = %d\n", cmdTable->numOfCommands);
//     printf("has Pipe ? %d\n", cmdTable->hasPipe);
//     printf("has Redirection ? %d\n", cmdTable->hasRedirection);
//     printf("Current Position: %d\n", cmdTable->currentPosition);  

    if (!cmdTable) return;
    printf("\n============== Current Command ==============\n");
    
    printf("Command\t\tpipeN \n");
    for(int i = 0 ; i < cmdTable->numOfCommands; i++){
        int cur = (cmdTable->currentPosition + i);
            for(int j = 0; cmdTable->cmdTable[cur][j]; j++)
                printf("%s\t", cmdTable->cmdTable[cur][j]);
        printf("%d\n", cmdTable->pipeN[cur]);
    }


}

/*
    name: printTable
    description: print the command table
*/
static void printTable(char*** pipetable)
{
    char ***stringArr = pipetable;
    while(**stringArr){
        char **tmp = *stringArr;
        while(*tmp)
            printf("%s ", *tmp++);
        printf("\n");
        stringArr++;
    }
}

/*
    name: freeCommand
    description: free memory
*/

static void freeCommand(CommandTable* cmdTable)
{
    for(int i = 0 ; i < cmdTable->row; i++){
        for(int j = 0; cmdTable->cmdTable[i][j]; j++){
            free(cmdTable->cmdTable[i][j]);
        }
        free(cmdTable->cmdTable[i]);
    }
    free(cmdTable->cmdTable);
    free(cmdTable);
}

/*
    name: tokenizer
    description: split the string
*/

static int tokenizer(char *line, char* delim, char** destination, int size)
{
    char *token;
    int pos = 0;
    token = strtok(line, delim);
    while (token != NULL) {
        destination[pos++] = strdup(token);
        token = strtok(NULL, delim);
    }
    
    destination[pos] = NULL;
    return pos;
}
