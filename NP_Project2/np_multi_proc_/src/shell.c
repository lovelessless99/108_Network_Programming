#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>

#include "CommandTable.h"
#include "history.h"
#include "socket.h"
#include "global.h"
#include "shell.h"
#include "shamem.h"
#include "client.h"

int clientfd;
pid_t clientpid;

static void read_line(char**, int);
static int launch(int, pid_t);
static void printenv(char *);
static void who(pid_t);


static void tell(int, CommandTable *);
static void yell(int, CommandTable *);
static int checkName(pid_t pid, char* name);
static void changeName(int clientfd, CommandTable *cmdTable);

static void createFIFO(int from_id, int to_id);
static char* getFIFONAME(int from_id, int to_id);

static void execute(CommandTable *);
static void execute_pipeline(CommandTable *);


void init_shell(struct myShell** shell)
{
    (*shell) = malloc(sizeof(struct myShell));
    (*shell)->launch = launch;
    (*shell)->printenv = printenv;
    (*shell)->execute = execute;
    (*shell)->execute_pipeline = execute_pipeline;
}


/*
    name: read_line
    description: read user input
*/
static void read_line(char** buffer, int buffersize)
{
   int pos = 0;
    while(pos < buffersize)
    {
        char c;
        read(clientfd, &c, 1);
        if(c == '\n' || c == '\r'){
            (*buffer)[pos] = '\0';
            return;
        }

        else if (c == EOF){ return;}
        else{ (*buffer)[pos++] = c; }
    }
}

/*
    name: launch
    description: launch a shell program and input command line
*/
static int launch(int client_fd, pid_t client_pid)
{
    
    client* me = get_client_by_pid(client_pid);
    setenv("PATH", me->path, 1);
    clientfd = client_fd;
    clientpid = client_pid;
    
    CommandTable *cmdTable = NULL;
    init_CommandTable(&cmdTable, TOKEN_BUFFSIZE, CMD_PARAM, me->id, me->pid);
    int buffersize = BUFFSIZE;

    while(1){
        char *line = malloc(sizeof(char) * buffersize);
        bzero(line,buffersize); 
        read_line(&line, buffersize);
        if(line == NULL) break;
        if(!strcmp(line, "")) continue; 
        if(!strcmp(line, "exit")) return -1;
        cmdTable->set_Command_Table(line, &cmdTable);
        execute(cmdTable);
        cmdTable->currentPosition = (cmdTable->currentPosition + cmdTable->numOfCommands);
        free(line);
        write(clientfd, "% ", 2);
    }
    return 1;
}


/*
    name: printenv
    description: print the envirnment variable
*/
static void printenv(char *argv)
{
  if(argv){
      char *env = getenv(argv);
      write(clientfd, env, strlen(env));
      write(clientfd, "\n", 1);
  }
  else{
      dup2(clientfd, STDERR_FILENO);
      fprintf(stderr, "Please Input Envirnment Name!\n");
  }
} 


static void who(pid_t pid)
{
    
	memset(share_message, 0, sizeof(share_message));

    sprintf(share_message, "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n");
    
    for (int i = 1; i <= MAX_CLIENTS; ++i)
    {
		if (client_info[i].enable)
		{
            char temp[500] = {0};
			            
            if (client_info[i].pid == pid) {
                sprintf(temp, "%d\t%s\t%s:%u\t<-me\n", client_info[i].id, client_info[i].name, client_info[i].ip_address, (unsigned int) client_info[i].port);
            }            
            else { 
                sprintf(temp, "%d\t%s\t%s:%u\n", client_info[i].id, client_info[i].name, client_info[i].ip_address, (unsigned int) client_info[i].port);
            }

            
            
            strncat(share_message, temp, strlen(temp));
        }
    }
	kill(pid, SIGUSR1);
}


static void tell(int clientfd, CommandTable *cmdTable)
{
    #define Command(N) cmdTable->cmdTable[cmdTable->currentPosition][N]
    
    int from_fd = clientfd;
    int to_id = atoi(strdup(Command(1)));
    char tell_message[5000] = {0};
    for(int i = 2; Command(i); i++)
    {
        strcat(tell_message, Command(i));
        strcat(tell_message, " ");
    }
    

    int found = 0;
    char buffer[50] = {0};

    client *sender = get_client_by_id(cmdTable->self_id);
    client *rec    = get_client_by_id(to_id);

    if (rec != NULL && rec->enable > 0)
    {
        found = 1;
    }

    if (found)
    {
        memset(share_message, 0, sizeof(char) * BUFFSIZE);
        sprintf(share_message, "*** %s told you ***: %s\n", sender->name, tell_message);
        kill(rec->pid, SIGUSR1);
		usleep(500);
    } 
    else
    {
        sprintf(buffer, "*** Error: user #%d does not exist yet. ***\n", to_id);
        send(from_fd, buffer, strlen(buffer), 0);
    }


}


static void yell(int clientfd, CommandTable *cmdTable)
{
    #define Command(N) cmdTable->cmdTable[cmdTable->currentPosition][N]

    char yell_message[5000] = {0};
    for(int i = 1; cmdTable->cmdTable[cmdTable->currentPosition][i] ; i++)
    {
        strcat(yell_message, cmdTable->cmdTable[cmdTable->currentPosition][i]);
        if(cmdTable->cmdTable[cmdTable->currentPosition][i+1]) 
            strcat(yell_message, " ");
    }

    if(cmdTable->hasRedirection)
    {
         strcat(yell_message, " > ");
         strcat(yell_message,  cmdTable->cmdTable[cmdTable->currentPosition + 1][0]);
    }

    broadcast(cmdTable->pid, YELL, yell_message, -1, -1);
}


static int checkName(pid_t pid, char* name)
{   
    for (int i = 1; i <= MAX_CLIENTS; ++i)
    {
        if ((client_info[i].pid != pid) && (!strcmp(client_info[i].name, name)))
        {
            return -1;
        }
    }
    return 1;
}

static void changeName(int clientfd, CommandTable *cmdTable)
{
    #define Command(N) cmdTable->cmdTable[cmdTable->currentPosition][N]
    char *new_name;
    new_name =  strdup(Command(1));
    
    //client *client_ptr = get_client(clientfd);
    client *client_ptr = get_client_by_pid(cmdTable->pid);

    if (checkName(client_ptr->pid, new_name) > 0)
    {
        memset(client_ptr->name, 0, sizeof(client_ptr->name));
        strncpy(client_ptr->name, new_name, strlen(new_name));
        broadcast(cmdTable->pid, CHANGENAME, new_name, -1, -1);
    }
    else
    {
        char temp[BUFFSIZE] = {0};
        sprintf(temp, "*** User '%s' already exists. ***\n", new_name);
        send(clientfd, temp, strlen(temp), 0);
    }
}





/*
    name: execute
    description: execute the corrspond command based on command style and length
*/
static void execute(CommandTable *cmdTable)
{
    #define Command(N) cmdTable->cmdTable[cmdTable->currentPosition][N]

    if(!strcmp(Command(0), "setenv"))
    {
        setenv(Command(1), Command(2), 1);
    }
    
    else if(!strcmp(Command(0), "printenv"))
    {
        printenv(Command(1));
    } 

    else if(!strcmp(Command(0), "who"))
    {
        who(cmdTable->pid);
    }

    else if(!strcmp(Command(0), "tell"))
    {
        tell(clientfd, cmdTable);
    }

    else if(!strcmp(Command(0), "yell"))
    {
        yell(clientfd, cmdTable);
    }

    else if(!strcmp(Command(0), "name"))
    {
        changeName(clientfd, cmdTable);
    }

    else
    {
         if(cmdTable->hasReceive)
        {
            char temp[100] = {0}; 
            if (get_client_by_id(cmdTable->from_id) == NULL)
            {
                sprintf(temp, "*** Error: user #%d does not exist yet. ***\n", cmdTable->from_id);
                send(clientfd, temp, strlen(temp), 0);
                return ;
            }
            
            else if (user_pipe[cmdTable->from_id][cmdTable->self_id][0] == 0)
            {
                sprintf(temp, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", cmdTable->from_id, cmdTable->self_id);
                send(clientfd, temp, strlen(temp), 0);
                return;
            }
            
            else
            {
                broadcast(clientpid, RECEIVEPIPE, cmdTable->line, cmdTable->from_id, cmdTable->self_id);
            }

        }

        if(cmdTable->hasSender)
        {
            if(cmdTable->hasReceive)
                usleep(1000);

            char temp[100] = {0}; 
            if (get_client_by_id(cmdTable->to_id) == NULL)
            {
                sprintf(temp, "*** Error: user #%d does not exist yet. ***\n", cmdTable->to_id);
                send(clientfd, temp, strlen(temp), 0);
                return;
            }
            
            
            else if (user_pipe[cmdTable->self_id][cmdTable->to_id][0] == 1)
            {
                sprintf(temp, "*** Error: the pipe #%d->#%d already exists. ***\n", cmdTable->self_id, cmdTable->to_id);
                send(clientfd, temp, strlen(temp), 0);
                return;
            }

            else
            {
                broadcast(clientpid, SENDPIPE, cmdTable->line, -1, cmdTable->to_id);
            }


        }

        execute_pipeline(cmdTable);
    }

}

static char* getFIFONAME(int from_id, int to_id)
{
    const char *directory = "./user_pipe/userpipe";
    // const char *directory = "userpipe";
    char from[10] = {0};
    char to[10] = {0};

    sprintf(from, "%d", from_id);
    sprintf(to  , "%d", to_id);
    
    char *str = NULL;     
    append(&str, directory, strlen(directory));
    append(&str, from, strlen(from));
    append(&str, to  , strlen(to));
    return str;
}

static void createFIFO(int from_id, int to_id)
{
	//const char* directory_name = "./user_pipe";
    /*
     * if(mkdir(directory_name, 0777) == -1)
        fprintf(stderr, "directory: %s exist!\n", directory_name);
    */

    const char *directory = "./user_pipe/userpipe";
    
    
    char from[10] = {0};
    sprintf(from, "%d", from_id);
      
    char *str = NULL;     
    char to[10] = {0};
    append(&str, directory, strlen(directory));
    sprintf(to  , "%d", to_id);
    append(&str, from, strlen(from));
    append(&str, to  , strlen(to));
              
    mkfifo(str,  0666) ;    

    free(str);
}




/*
    name: execute_pipeline
    description: execute the commands
*/

static void execute_pipeline(CommandTable *cmd_table)
{
    int numOfCommands = (cmd_table->hasRedirection)? cmd_table->numOfCommands-1 : cmd_table->numOfCommands;
     
    pid_t pid;

    char *sendFIFO = NULL, *receiveFIFO = NULL;
    for(int i = 0 ; i < numOfCommands; i++)
    {
       
        
        int cur  = (cmd_table->currentPosition + i );
        int next = (cur +  cmd_table->pipeN[cur] );
    
        int in = STDIN_FILENO;
        int out = clientfd;
    
        dup2(clientfd, STDERR_FILENO);

        if(cmd_table->pipetable[cur][0] != STDIN_FILENO){
            in = cmd_table->pipetable[cur][0];
            close(cmd_table->pipetable[cur][1]);
        }
        
        
        else if (user_pipe[cmd_table->from_id][cmd_table->self_id][0])
        {
            user_pipe[cmd_table->from_id][cmd_table->self_id][0] = 0;
        }
        
        
        
        if(cmd_table->pipeN[cur] > 0){
            if(cmd_table->pipetable[next][1] == STDOUT_FILENO){ pipe(cmd_table->pipetable[next]);}
            out = cmd_table->pipetable[next][1];
        }
        
        
        else if(i+1 == numOfCommands && cmd_table->hasSender)
        {
            if(cmd_table->hasSender)
                createFIFO(cmd_table->self_id, cmd_table->to_id);
            user_pipe[cmd_table->self_id][cmd_table->to_id][0] = 1;
        }
        
        

        while((pid=fork()) < 0)
        {
            //while(waitpid(pid, NULL,WNOHANG) != -1);
            while(waitpid(-1, NULL, WNOHANG) > 0);
        }
        
        if(pid == 0){ 
            
            dup2(clientfd, STDERR_FILENO);

            
            
            if(i == 0 && cmd_table->hasReceive)
            {
                receiveFIFO = getFIFONAME(cmd_table->from_id, cmd_table->self_id);
                in = open(receiveFIFO, O_RDONLY);
            }

        
        
            if(i+1 == numOfCommands && cmd_table->hasSender)
            {
                sendFIFO = getFIFONAME(cmd_table->self_id, cmd_table->to_id);
                out = open(sendFIFO, O_WRONLY); 
            }
            

            if(in != STDIN_FILENO){
                dup2(in, STDIN_FILENO);
                close(in);
            }

            if(out != STDOUT_FILENO){ 
                dup2(out, STDOUT_FILENO);
                if(cmd_table->stderrTable[cur])
                    dup2(out, STDERR_FILENO);
            }
            
            
            if(cmd_table->hasRedirection){
                if(i+1 == numOfCommands){
                    char* filename = cmd_table->cmdTable[next][0];
                    int mode = (cmd_table->hasAppendRediction)?  O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
                    int permission = S_IRUSR | S_IWUSR;
                    int fd = open(filename, mode, permission);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }
            
            if(execvp(cmd_table->cmdTable[cur][0], cmd_table->cmdTable[cur]) < 0 ){
                dup2(clientfd, STDERR_FILENO);
                fprintf(stderr, "Unknown command: [%s].\n", cmd_table->cmdTable[cur][0]); 
            }
            exit(EXIT_FAILURE);
        }

        else{
            if(in != STDIN_FILENO){ close(in);}
            
            if(!cmd_table->hasSender){ 
                if(out != clientfd){ 
                    int finish_pre = 1;
                    for(int k = 0 ; k <= cur && finish_pre; k++){
                        if(cmd_table->pipeN[k])
                           finish_pre = 0;
                    }
                
                    if(finish_pre) close(out);
                }
            }
        }

        if(i+1 == numOfCommands && (cmd_table->pipeN[cur] == 0 || cmd_table->hasRedirection) && !cmd_table->hasSender)
        {    
            int status;
            waitpid(pid, &status, 0);
        }
        
        cmd_table->pipeN[cur] = 0;
        
    }
    

}
