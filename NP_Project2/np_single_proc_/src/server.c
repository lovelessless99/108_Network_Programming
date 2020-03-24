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
                                        int client_fd = connect_client(&user_list, socket_fd ,port);
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
                                                delete_client(&user_list, fd);
                                                close(fd);
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

int connect_client(client** user_list, int socket_fd, char* port)
{
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);
        int clientfd  = accept(socket_fd , &client_addr , &client_len);
        client* new_user = create_client(clientfd, get_IP_String((struct sockaddr *)&client_addr), port);
        insert_client(user_list, &new_user);
        return clientfd;
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