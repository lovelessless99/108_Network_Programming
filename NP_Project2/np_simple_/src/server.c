#include "server.h"

void server(char *port)
{
        int socket_fd = create_socket(port);
        int client_fd = connect_client(socket_fd);
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

int connect_client(int socket_fd)
{
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int clientfd  = accept(socket_fd , (struct sockaddr *)&client_addr , &client_len);
        // printf("client ip address = %s\n", get_IP_String((struct sockaddr *)&client_addr));
        return clientfd;
}

static char* get_IP_String(const struct sockaddr *sa)
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