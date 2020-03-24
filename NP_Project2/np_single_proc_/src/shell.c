#include "client.h"
#include "shell.h"
#include "tube.h"

client** user_list;
Tube** tube_list;
int pipe_table[PIPE_NUMBER][2];
int client_fd;
int count;

int launch(int clientfd, client** userlist, Tube** tubelist)
{
        client_fd = clientfd;
        user_list = userlist;
        tube_list = tubelist;
        
        client *me;
        for( me = *user_list; me && me->fd != client_fd; me = me->next_client );

        setenv("PATH", me->env_path, 1); // To Do, change by info
        
        char cmd[BUFFSIZE] = {0};        
        read(client_fd, cmd, BUFFSIZE);
        REMOVE_ENTER_CHAR(cmd); // Remove the end of command \r\n from client 
        
        if(!strcmp(cmd, "exit")) return -1;
        switch_command(cmd);
        write(client_fd, "% ", 2);
        return 1;
}

void switch_command(char *cmd)
{       
        char copy_cmd[BUFFSIZE] = {0};
        strcpy(copy_cmd, cmd);

        char *instruction = strtok(copy_cmd , SPACE);
        if(!strcmp(instruction, "printenv"))
        {
                char message[BUFFSIZE] = {0}; 
                char *tok = strtok(NULL, SPACE);
                if(tok){ 
                        if (getenv(tok)){ 
                                sprintf(message,"%s\n", getenv(tok));
                                write(client_fd, message, strlen(message));
                        }
                }
                else{ 
                        strcpy(message, "Please Input Envirnment Name!\n");
                        write(client_fd, message, strlen(message));
                }
        }
        
        else if(!strcmp(instruction, "setenv"))
        {
                char *name, *value;
                // tok   = strtok(cmd , SPACE);
                name  = strtok(NULL, SPACE);
                value = strtok(NULL, SPACE);

                if(name && value) { 
                        setenv(name, value, 1);
                        client *me;
                        for( me = *user_list; me && me->fd != client_fd; me = me->next_client );
                        free(me->env_path);
                        me->env_path = strdup(value);
                }
        }

        else if(!strcmp(instruction, "who"))  { who(); }
        else if(!strcmp(instruction, "tell")) { 
                char *receiver_id,*temp, message[BUFFSIZE] = {0};
                receiver_id  = strtok(NULL, SPACE);
                temp = strtok(NULL, SPACE);

                // concat the message
                while(temp != NULL)
                {
                        strcat(message, temp);
                        strcat(message, " ");
                        temp = strtok(NULL, SPACE);
                }
                tell(message, atoi(receiver_id));
        }

        else if(!strcmp(instruction, "yell")) { 
                char *temp, message[BUFFSIZE] = {0};
                client *sender = NULL;
                for(sender   = *user_list; sender   && sender->fd   != client_fd  ; sender   = sender->next_client   );
                sprintf(message, "*** %s yelled ***: ", sender->name);
                
                temp = strtok(NULL, SPACE);
                // concat the message
                while(temp != NULL)
                {
                        strcat(message, temp);
                        temp = strtok(NULL, SPACE);
                        if(temp) strcat(message, " ");
                }
                strcat(message, "\n");
                for_each_client(*user_list) { write(ptr->fd, message, strlen(message)); }
        }

        else if(!strcmp(instruction, "name")) { 
                char *temp, name[BUFFSIZE] = {0};
                temp = strtok(NULL, SPACE);
                
                // concat the message
                while(temp != NULL)
                {
                        strcat(name, temp);
                        temp = strtok(NULL, SPACE);
                        if(temp) strcat(name, " ");
                }
                changeName(name);
        }

        else { handler(cmd); }
}

void who()
{
        char message[5000] = {0};
        sprintf(message, "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n");

        for_each_client(*user_list)
        {
                char temp[500] = {0};
                if (ptr->fd == client_fd) { sprintf(temp, "%d\t%s\t%s:%s\t<-me\n", ptr->id, ptr->name, ptr->ip, ptr->port); }            
                else { sprintf(temp, "%d\t%s\t%s:%s\n", ptr->id, ptr->name, ptr->ip, ptr->port); }
                strcat(message, temp);
        }

        write(client_fd, message, strlen(message));
}

void tell(char *message, int receiver_id)
{
        char buffer[BUFFSIZE] = {0};
        client *sender, *receiver;

        // search
        for(sender   = *user_list; sender   && sender->fd   != client_fd  ; sender   = sender->next_client   );
        for(receiver = *user_list; receiver && receiver->id != receiver_id; receiver = receiver->next_client );
        
        if (receiver != NULL)
        {
                sprintf(buffer, "*** %s told you ***: %s\n", sender->name, message);
                write(receiver->fd, buffer, strlen(buffer));
        }
        else
        {
                sprintf(buffer, "*** Error: user #%d does not exist yet. ***\n", receiver_id);
                write(client_fd, buffer, strlen(buffer));
        }
}

void changeName(char* new_name)
{
        char message[BUFFSIZE] = {0};
        for_each_client(*user_list)
        {
                if(!strcmp(ptr->name, new_name))
                {
                        sprintf(message, "*** User '%s' already exists. ***\n", new_name);
                        write(client_fd, message, strlen(message));
                        return;
                }
        }

        client* user = NULL;
        for( user = *user_list; user && user->fd != client_fd ; user = user->next_client );
        free(user->name);
        user->name = strdup(new_name);

        sprintf(message, "*** User from %s:%s is named '%s'. ***\n", user->ip, user->port, user->name);
        for_each_client(*user_list) { write(ptr->fd, message, strlen(message)); }
}


void handler(char *line)
{
        bool receive_pipe, send_pipe = false;
        int send_peer_id, receive_peer_id;
        
        client *me;
        for( me = *user_list; me && me->fd != client_fd; me = me->next_client );

        char copy_cmd[BUFFSIZE] = {0};
        strcpy(copy_cmd, line);
        char *tok; tok = strtok(copy_cmd, SPACE);

        while(tok != NULL)
        {       
                char temp[100] = {0};

                Command cmd = {
                        .stdin  = 0,
                        .stdout = client_fd,
                        .stderr = client_fd,
                        .isWait = false
                };

                if(pipe_table[count][0] != 0)
                {
                        cmd.stdin = pipe_table[count][0];
                        close(pipe_table[count][1]);
                        pipe_table[count][1] = 0;
                }

                int argc = 0;

                /* Initialize Command Array */
                char **argv = malloc(sizeof(char*) * CMD_COUNT);
                memset(argv, 0, sizeof(char*) * CMD_COUNT);

                for(int i = 0 ; i < CMD_LENGTH; i++){ 
                        argv[i] = (char*)malloc(sizeof(char) * CMD_LENGTH);
                        memset(argv[i], 0, sizeof(char) * CMD_LENGTH);
                }
                
                while(tok && !strchr(PIPE_SYMBLE, tok[0]))
                {
                        strcpy(argv[argc++], tok);
                        tok = strtok(NULL, SPACE);
                }

                argv[argc] = NULL; /* Must do ! For execvp null terminate string array*/
                if(tok == NULL) { cmd.isWait = true; }

                while(tok && strchr(PIPE_SYMBLE, tok[0]))
                {
                        int to;

                        switch(tok[0]) {
                                case '|':
                                        if( strlen(tok) == 1 ) { to = 1; }
                                        else { sscanf(tok, "|%d", &to); }
                                        if (pipe_table[count + to][1] == 0) { 
                                                pipe(pipe_table[count + to]); 
                                        }
                                        cmd.stdout = pipe_table[count + to][1];
                                break;

                                case '!':
                                        if( strlen(tok) == 1 ) { to = 1; }
                                        else { sscanf(tok, "!%d", &to); }
                                        if (pipe_table[count + to][1] == 0) { 
                                                pipe(pipe_table[count + to]); 
                                        }
                                        cmd.stdout = cmd.stderr = pipe_table[count + to][1]; 
                                        
                                break;

                                case '>':
                                        // send pipe
                                        if( strlen(tok) > 1 && strcmp(tok, ">&") != 0 )
                                        {
                                               
                                                printList(*tube_list);
                                                send_pipe = true;
                                                sscanf(tok, ">%d", &send_peer_id);
                                                client *receiver;
                                                Tube* tube;
                                                for(receiver = *user_list; receiver && receiver->id != send_peer_id; receiver = receiver->next_client );
                                                for(tube = *tube_list; tube && ( tube->sender != me->id || tube->receiver != send_peer_id ) ; tube = tube->next_tube);

                                                if (receiver == NULL) { 
                                                        sprintf(temp, "*** Error: user #%d does not exist yet. ***\n", send_peer_id);
                                                        write(client_fd, temp, strlen(temp));
                                                        return; 

                                                }
                                                
                                                if(tube != NULL) { 
                                                        sprintf(temp, "*** Error: the pipe #%d->#%d already exists. ***\n", me->id, send_peer_id);
                                                        write(client_fd, temp, strlen(temp));
                                                        return; 
                                                }
                                                
                                                // create tube and broadcast
                                                tube = create_tube(me->id, send_peer_id);
                                                push_tube(tube_list, &tube);
                                                sprintf(temp, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", me->name, me->id, line, receiver->name, receiver->id);
                                                cmd.stdout = tube->pipe_fd[1];
                                                
                                                for_each_client(*user_list) { write(ptr->fd, temp, strlen(temp)); }
                                                printList(*tube_list);
                                                break;     
                                        }

                                        cmd.isWait = true;
                                        tok = strtok(NULL, SPACE); 
                                        int file_fd = open(tok, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                                        if(!strcmp(tok, ">&")){ cmd.stderr = file_fd; }
                                        else{ cmd.stdout = file_fd; }
                                        
                                break;

                                case '<':
                                        // receive pipe
                                        receive_pipe = true;
                                        sscanf(tok, "<%d", &receive_peer_id);
                                        client *sender;
                                        Tube* tube;

                                        for(sender = *user_list; sender && sender->id != receive_peer_id; sender = sender->next_client);
                                        for(tube = *tube_list; tube && ( tube->sender != receive_peer_id || tube->receiver != me->id ) ; tube = tube->next_tube);
                                        if (sender == NULL) {
                                                sprintf(temp, "*** Error: user #%d does not exist yet. ***\n", receive_peer_id);
                                                write(client_fd, temp, strlen(temp));
                                                return;
                                        }

                                        if (tube == NULL){
                                                sprintf(temp, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", receive_peer_id, me->id);
                                                write(client_fd, temp, strlen(temp));
                                                return;
                                        }

                                        cmd.stdin = tube->pipe_fd[0];
                                        sprintf(temp, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", me->name, me->id, sender->name, sender->id, line);
                                        for_each_client(*user_list) { write(ptr->fd, temp, strlen(temp)); }
                                        printList(*tube_list);


                                break;
                        }
                        tok = strtok(NULL, SPACE);
                }

                pid_t pid;
                switch (pid = fork()){
                        case -1:
                                while(waitpid(-1, NULL, WNOHANG) > 0);
                                break;
                        
                        case 0:
                                if (cmd.stdin  != 0) { dup2(cmd.stdin , STDIN_FILENO) ; }
                                if (cmd.stdout != 1) { dup2(cmd.stdout, STDOUT_FILENO); }
                                if (cmd.stderr != 2) { dup2(cmd.stderr, STDERR_FILENO); }

                                if( execvp(argv[0], argv) < 0 )
                                {
                                        fprintf(stderr, "Unknown command: [%s].\n", argv[0]);
                                }
                                exit(EXIT_FAILURE);

                        default:
                                if (pipe_table[count][0] != 0) {
                                        close(pipe_table[count][0]);
                                        pipe_table[count][0] = 0;
                                }
                                // close file 
                                
                                if(receive_pipe == true) { delete_id_tube(tube_list, receive_peer_id, me->id);}
                                if (cmd.isWait) { waitpid(pid, NULL, 0); }
                                for(int i = 0 ; i < CMD_LENGTH; i++){ free(argv[i]); }
                                free(argv);

                }
                ++count;
        }
}