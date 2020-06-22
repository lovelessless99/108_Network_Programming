// #include "server.h"
// #include "client.h"
// #include "shell.h"
// #include "tube.h"

// client* user_list = NULL;
// Tube*   tube_list = NULL;

// void server(char *port)
// {       
//         int socket_fd = create_socket(port);
//         // int fdmax = socket_fd;
//         // fd_set main_fdset, copy_fdset;
        
//         // FD_ZERO(&main_fdset);
//         // FD_ZERO(&copy_fdset);
//         // FD_SET(socket_fd, &main_fdset);

        
//         // while(true)
//         // {
//         //         memcpy(&copy_fdset, &main_fdset, sizeof(main_fdset));
//         //         while (select(fdmax + 1, &copy_fdset, NULL, NULL, NULL) == -1);
//         //         for(int fd = 0; fd <= fdmax; fd++)
//         //         {
//         //                 if(FD_ISSET(fd, &copy_fdset))
//         //                 {
//         //                         if (fd == socket_fd)
//         //                         {
                                        
//         //                                 FD_SET(client_fd, &main_fdset);
//         //                                 if (client_fd > fdmax) { fdmax = client_fd; }
//         //                                 write(client_fd, WELCOME, strlen(WELCOME));
//         //                                 write(client_fd, "% ", 2);
//         //                         }
                        
//         //                         else
//         //                         {
//         //                                 clearenv();
//         //                                 int status = launch(fd, &user_list, &tube_list);
//         //                                 if(status == -1)
//         //                                 {
//         //                                         FD_CLR(fd, &main_fdset);
//         //                                         free_resource(&user_list, &tube_list, fd);
//         //                                 }         
//         //                         }
//         //                 }
//         //         }
//         // }

//         while (1)
// 	{
//                 int client_fd = connect_client(&user_list, socket_fd);
// 		if ( client_fd <= 0) { continue; }

// 		write(client_fd, WELCOME, strlen(WELCOME));
                
//                 pid_t pid = fork() ;
//                 switch(pid){
//                         case -1 : continue;
//                         case  0 : close(socket_fd);
                                  
//                         default : close(client_fd);
//                 }
//                 if (pid < 0)
//                 {
// 				continue;
// 			}
// 			if (pid == 0)
// 			{
// 				close(socket_fd);
// 				init_client(client_fd, getpid());
// 				broadcast(getpid(), LOGIN, NULL, -1, -1);
// 				write(clientfd, "% ", 2);
// 				shell->launch(clientfd, getpid());
// 				broadcast(getpid(), LOGOUT, NULL, -1, -1);
//                 		reset_client(getpid());
// 				exit(0);
// 			}
// 			else
// 			{
// 				close(clientfd);
// 			}
// 		}
// 		else
// 		{
// 			client_count--;
// 		}
// 	}

// }

// int create_socket(char *port)
// {
//         int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//         if(socket_fd == -1) { goto SOCKET_ERROR; }

//         int option = SO_REUSEADDR;
//         if ( setsockopt(socket_fd, SOL_SOCKET, option, &option, sizeof(option)) == -1 ) { goto SETSOCKOPT_ERROR; }
        
//         struct sockaddr_in server_addr = {
//                 .sin_addr.s_addr = htonl(INADDR_ANY),
//                 .sin_port        = htons(atoi(port)),
//                 .sin_family      = AF_INET,
//         };

//         if( bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ) { goto BIND_ERROR; }

//         if( listen(socket_fd, REQUEST_QUEUE_LEN) == -1 ) { goto LISTEN_ERROR; }

//         return socket_fd;


//         SOCKET_ERROR:
//                 fprintf(stderr, "socket create error!\n");
//                 return -1;
        
//         SETSOCKOPT_ERROR:
//                 fprintf(stderr, "setsockopt error!\n");
//                 return -1;

//         BIND_ERROR:
//                 fprintf(stderr, "binding error!\n");
//                 return -1;
        
//         LISTEN_ERROR:
//                 fprintf(stderr, "listening error!\n");
//                 return -1;
// }

// int connect_client(client** user_list, int socket_fd)
// {
//         struct sockaddr client_addr;
//         socklen_t client_len = sizeof(client_addr);
//         int clientfd  = accept(socket_fd , &client_addr , &client_len);

//         char port[10] = {0};
//         sprintf(port, "%u", (uint16_t)ntohs( ((struct sockaddr_in*)(&client_addr))->sin_port));
//         client* new_user = create_client(clientfd, get_IP_String((struct sockaddr *)&client_addr), port);
        
//         char message[100] = {0};
//         sprintf(message, "*** User '%s' entered from %s:%s. ***\n", new_user->name, new_user->ip, new_user->port);
//         for_each_client(*user_list){ write(ptr->pid, message, strlen(message)); }
//         insert_client(user_list, &new_user);
//         return clientfd;
// }

// void free_resource(client** user_list, Tube** tube_list ,int pid)
// {
//         client* remove_client;
//         for(remove_client = *user_list; remove_client->pid != pid; remove_client = remove_client->next_client);
//         char message[100] = {0};
//         sprintf(message, "*** User '%s' left. ***\n", remove_client->name);       
//         delete_client(user_list, pid);
//         delete_all_id_tube(tube_list, pid);
//         close(fd);
//         for_each_client(*user_list) { write(ptr->pid, message, strlen(message)); }
// }

// char* get_IP_String(const struct sockaddr *sa)
// {
//         char *ip;
//         switch(sa->sa_family) {
//                 case AF_INET: // ipv4
//                         ip = (char*) malloc(sizeof(char) * INET_ADDRSTRLEN);
//                         inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ip, INET_ADDRSTRLEN);
//                         return ip;

//                 case AF_INET6: // ipv6
//                         ip = (char*) malloc(sizeof(char) * INET6_ADDRSTRLEN);
//                         inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), ip, INET6_ADDRSTRLEN);
//                         return ip;

//                 default:
//                         fprintf(stderr, "Unknown AF!\n");
//                         return NULL;
//         }
// }

// void clear_shmem_signal()
// {
//         struct sigaction sa;
//         sa.sa_handler = free_share_mem;
//         sigemptyset(&sa.sa_mask);

//         if (sigaction(SIGINT , &sa , NULL) == -1)  perror("sigaction int");
//         if (sigaction(SIGQUIT, &sa , NULL) == -1)  perror("sigaction quit");
// }

// void broadcast_signal()
// {
//         struct sigaction sa;
// 	sa.sa_handler = sigusr1_handler;
// 	sigemptyset(&sa.sa_mask);
// 	sa.sa_flags = SA_RESTART;

// 	if (sigaction(SIGUSR1, &sa, NULL) == -1) { perror("sigaction sigusr1"); }
// }


// void sigusr1_handler()
// {
//         for_each_client(*user_list)
//         {

//         }
// // 	for (int i = 1; i <= MAX_CLIENTS; ++i)
// // 	{
// // 		9
// // 		{
// // 			send(client_info[i].client_fd, share_message, strlen(share_message), 0);
// // 			break;
// // 		}
// // 	}
// }

