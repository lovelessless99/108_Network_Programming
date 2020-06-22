#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "socket.h"
#include "client.h"
#include "shell.h"
#include "shamem.h"

void server(char* port)
{
    char welcome[300] = "****************************************\n"
                        "** Welcome to the information server. **\n"
                        "****************************************\n";

	int client_count = 0;
    int sockfd;
	
	create_socket(&sockfd, port);
    //createFIFO();
	register_signal();
	init_share_mem();
	init_client_info();
	recv_broadcast();


    pid_t pid;
	struct myShell* shell = NULL;
	init_shell(&shell);
	int clientfd;

	while (1)
	{
		if ((clientfd = connect_client(sockfd)) <= 0)
		{
			continue;
		}
		client_count++;
		if (client_count <= MAX_CLIENTS)
		{
			send(clientfd, welcome, strlen(welcome), 0);
			// success
			pid = fork();
			if (pid < 0)
			{
				continue;
			}
			if (pid == 0)
			{
				close(sockfd);
				init_client(clientfd, getpid());
				broadcast(getpid(), LOGIN, NULL, -1, -1);
				write(clientfd, "% ", 2);
				shell->launch(clientfd, getpid());
				broadcast(getpid(), LOGOUT, NULL, -1, -1);
                reset_client(getpid());
				exit(0);
			}
			else
			{
				close(clientfd);
			}
		}
		else
		{
			client_count--;
		}
	}
	free_share_mem();
}

void create_socket(int *sockfd, char* port)
{
    Addrinfo hints, *servinfo, *p;
	int rv, yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    
    
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(rv));
		exit(1);
	}
 


	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server : socket");
			continue;
		}

		if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
	    
        if(setsockopt(*sockfd , SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1)
        {
            perror("setsocketopt");
            exit(1);
        }    
		if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("server : bind");
			close(*sockfd);
			continue;
		}
	
	}
	freeaddrinfo(servinfo);
	
	if (listen(*sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}
	
}  

int connect_client(int sockfd)
{
	socklen_t sin_size;
	struct sockaddr_storage their_addr;
	int clientfd;
	sin_size = sizeof(their_addr);
	clientfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	
	return clientfd;	
}


void broadcast(pid_t client_pid, Option option, char *_message, int from_id, int to_id)
{
	client *client_ptr = get_client_by_pid(client_pid);
	client *from_clinet = get_client_by_id(from_id);
	client *send_client = get_client_by_id(to_id);
	memset(share_message, 0, sizeof(char) * BUFFSIZE);

	// char message[BUFFSIZE] = {0};

	switch(option)
	{
		case LOGIN:	
            sprintf(share_message, "*** User '%s' entered from %s:%u. ***\n", client_ptr->name, client_ptr->ip_address, client_ptr->port);
			break;
		case LOGOUT:
			sprintf(share_message, "*** User '%s' left. ***\n", client_ptr->name);
			break;
		case YELL:
			sprintf(share_message, "*** %s yelled ***: %s\n", client_ptr->name, _message);
			break;
		case CHANGENAME:
            sprintf(share_message, "*** User from %s:%u is named '%s'. ***\n", client_ptr->ip_address, client_ptr->port,  _message);
			break;
		case RECEIVEPIPE:
			sprintf(share_message, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", client_ptr->name, client_ptr->id, from_clinet->name, from_clinet->id, _message);
			break;
		case SENDPIPE:
            sprintf(share_message, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", client_ptr->name, client_ptr->id, _message, send_client->name, send_client->id);
			break;

	}

	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		// if (client_info[i].client_fd)
		// {
		// 	send(client_info[i].client_fd, message, strlen(message), 0);
		// }
		if (client_info[i].enable)
		{
			kill(client_info[i].pid, SIGUSR1);
			sleep(0.5);
		}
	}

}

void reset_client(pid_t client_pid)
{


	client *client_ptr = get_client_by_pid(client_pid);
	int client_id = client_ptr->id;
	
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (user_pipe[client_id][i][0])
		{
			close(user_pipe[client_id][i][0]);
		}
		if (user_pipe[client_id][i][1])
		{
			close(user_pipe[client_id][i][1]);
		}
	}
	memset(user_pipe[client_id], 0, sizeof(user_pipe[client_id]));
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (user_pipe[i][client_id][0])
		{
			close(user_pipe[i][client_id][0]);
		}
		if (user_pipe[i][client_id][1])
		{
			close(user_pipe[i][client_id][1]);
		}
		memset(user_pipe[i][client_id], 0, sizeof(user_pipe[i][client_id]));
	}

	close(client_ptr->client_fd);
	memset(client_ptr, 0, sizeof(client));
	
}



int append(char **str, const char *buf, int size)
{
    char *nstr;
    if(*str == NULL)
    {
        nstr = malloc(size + 1);
        memcpy(nstr, buf, size);
        nstr[size] = '\0';
    }
    
    else{
        if(asprintf(&nstr, "%s_%s", *str, buf) == -1)  return -1;
        free(*str);  
    }

    *str = nstr;
    return 0;
}

void clear_tmp_directory(const char* dir_name)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(dir_name);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(!strcmp(dir->d_name, ".") && !strcmp(dir->d_name, "..")){
               char *file_path = NULL;

               append(&file_path, dir_name   , strlen(dir_name   ));
               append(&file_path, "/"        , 1                  );
               append(&file_path, dir->d_name, strlen(dir->d_name));
               
               printf("%s\n", file_path);
               
               remove(file_path);
               if(unlink(file_path) == -1)
                    fprintf(stderr, "delete %s fail!\n", file_path);
               free(file_path);
            }
        }
        closedir(d);
    }
    if(rmdir(dir_name) == -1)
        fprintf(stderr, "remove directory %s failed!\n", dir_name);
}

void register_signal()
{
	struct sigaction sa;
    sa.sa_handler = free_share_mem;
    sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART;
    if(sigaction(SIGINT, &sa , NULL) == -1 || sigaction(SIGQUIT, &sa , NULL) == -1)
    {
        perror("sigaction");
	}
}


void recv_broadcast()
{
	struct sigaction sa;
	sa.sa_handler = sigusr1_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR1, &sa, NULL) == -1)
	{
		perror("sigaction sigusr1");
	}
}

void sigusr1_handler()
{
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (client_info[i].enable)
		{
			send(client_info[i].client_fd, share_message, strlen(share_message), 0);
			break;
		}
	}
}


