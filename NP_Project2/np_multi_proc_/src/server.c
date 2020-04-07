#include "server.h"
#include "client.h"
#include "shell.h"
#include "tube.h"

void server(char *port)
{       
        int socket_fd = create_socket(port);
        int fdmax = socket_fd;
        fd_set main_fdset, copy_fdset;
        
        FD_ZERO(&main_fdset);
        FD_ZERO(&copy_fdset);
        FD_SET(socket_fd, &main_fdset);

        client* user_list = NULL;
        Tube*   tube_list = NULL;
        while(true)
        {
                memcpy(&copy_fdset, &main_fdset, sizeof(main_fdset));
                while (select(fdmax + 1, &copy_fdset, NULL, NULL, NULL) == -1);
                for(int fd = 0; fd <= fdmax; fd++)
                {
                        if(FD_ISSET(fd, &copy_fdset))
                        {
                                if (fd == socket_fd)
                                {
                                        int client_fd = connect_client(&user_list, socket_fd);
                                        FD_SET(client_fd, &main_fdset);
                                        if (client_fd > fdmax) { fdmax = client_fd; }
                                        write(client_fd, WELCOME, strlen(WELCOME));
                                        write(client_fd, "% ", 2);
                                }
                        
                                else
                                {
                                        clearenv();
                                        int status = launch(fd, &user_list, &tube_list);
                                        if(status == -1)
                                        {
                                                FD_CLR(fd, &main_fdset);
                                                free_resource(&user_list, &tube_list, fd);
                                        }         
                                }
                        }
                }
        }
}

int create_socket(char *port)
{
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(socket_fd == -1) { goto SOCKET_ERROR; }

        int option = SO_REUSEADDR;
        if ( setsockopt(socket_fd, SOL_SOCKET, option, &option, sizeof(option)) == -1 ) { goto SETSOCKOPT_ERROR; }
        
        struct sockaddr_in server_addr = {
                .sin_addr.s_addr = htonl(INADDR_ANY),
                .sin_port        = htons(atoi(port)),
                .sin_family      = AF_INET,
        };

        if( bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ) { goto BIND_ERROR; }

        if( listen(socket_fd, REQUEST_QUEUE_LEN) == -1 ) { goto LISTEN_ERROR; }

        return socket_fd;


        SOCKET_ERROR:
                fprintf(stderr, "socket create error!\n");
                return -1;
        
        SETSOCKOPT_ERROR:
                fprintf(stderr, "setsockopt error!\n");
                return -1;

        BIND_ERROR:
                fprintf(stderr, "binding error!\n");
                return -1;
        
        LISTEN_ERROR:
                fprintf(stderr, "listening error!\n");
                return -1;
}

int connect_client(client** user_list, int socket_fd)
{
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);
        int clientfd  = accept(socket_fd , &client_addr , &client_len);

        char port[10] = {0};
        sprintf(port, "%u", (uint16_t)ntohs( ((struct sockaddr_in*)(&client_addr))->sin_port));
        client* new_user = create_client(clientfd, get_IP_String((struct sockaddr *)&client_addr), port);
        
        char message[100] = {0};
        sprintf(message, "*** User '%s' entered from %s:%s. ***\n", new_user->name, new_user->ip, new_user->port);
        for_each_client(*user_list){ write(ptr->fd, message, strlen(message)); }
        insert_client(user_list, &new_user);
        return clientfd;
}

void free_resource(client** user_list, Tube** tube_list ,int fd)
{
        client* remove_client;
        for(remove_client = *user_list; remove_client->fd != fd; remove_client = remove_client->next_client);
        char message[100] = {0};
        sprintf(message, "*** User '%s' left. ***\n", remove_client->name);       
        delete_client(user_list, fd);
        delete_all_id_tube(tube_list, fd);
        close(fd);
        for_each_client(*user_list) { write(ptr->fd, message, strlen(message)); }
}


char* get_IP_String(const struct sockaddr *sa)
{
        char *ip;
        switch(sa->sa_family) {
                case AF_INET: // ipv4
                        ip = (char*) malloc(sizeof(char) * INET_ADDRSTRLEN);
                        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ip, INET_ADDRSTRLEN);
                        return ip;

                case AF_INET6: // ipv6
                        ip = (char*) malloc(sizeof(char) * INET6_ADDRSTRLEN);
                        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), ip, INET6_ADDRSTRLEN);
                        return ip;

                default:
                        fprintf(stderr, "Unknown AF!\n");
                        return NULL;
        }
}