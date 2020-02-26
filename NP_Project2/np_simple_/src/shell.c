#include "shell.h"

int pipe_table[PIPE_NUMBER][2];
int count;

void readline(char** cmd)
{
        char buffer[BUFFSIZE] = {0};
        int pos = 0;

        while(true)
        {
                int c = getchar();
                if(c == '\n' || c == '\r')
                {
                        buffer[pos] = '\0';
                        *cmd = strdup(buffer);
                        return;
                }
                else if (c == EOF) { return;}
                else { buffer[pos++] = c; }
        }
}

void launch(void)
{
        setenv("PATH", "bin:.", 1);
        while(true)
        {
                char *cmd;
                printf("%% ");
                readline(&cmd);
                if(!strcmp(cmd, "")) continue;
                switch_command(cmd);
                free(cmd);
        }
}

void switch_command(char *cmd)
{
        if(!strcmp(cmd, "exit")) { exit(EXIT_SUCCESS); }

        else if(strstr(cmd, "printenv"))
        {
                char *tok;
                tok = strtok(cmd , SPACE);
                tok = strtok(NULL, SPACE);
                if(tok){ 
                        if (getenv(tok)){ printf("%s\n", getenv(tok)); }
                }
                else{ fprintf(stderr, "Please Input Envirnment Name!\n"); }
        }

        else if(strstr(cmd, "setenv"))
        {
                char *tok, *name, *value;
                tok   = strtok(cmd , SPACE);
                name  = strtok(NULL, SPACE);
                value = strtok(NULL, SPACE);

                if(name && value) { setenv(name, value, 1); }
        }

        else { handler(cmd); }
}

void handler(char *line)
{
        
        char *tok;
        tok = strtok(line, SPACE);

        while(tok != NULL)
        {       
                Command cmd = {
                        .stdin  = 0,
                        .stdout = 1,
                        .stderr = 2,
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
                                        cmd.isWait = true;
                                        if (strcmp(tok, ">&") == 0) {
                                                tok = strtok(NULL, SPACE); 
                                                cmd.stderr = open(tok, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                                        } // error output
                                        
                                        else { 
                                                tok = strtok(NULL, SPACE);
                                                cmd.stdout = open(tok, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                                        } // standard output
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
                                        
                                if (cmd.isWait) { waitpid(pid, NULL, 0); }

                                for(int i = 0 ; i < CMD_LENGTH; i++){ free(argv[i]); }
                                free(argv);

                }
                ++count;
        }
}