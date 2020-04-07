#include "sock_server.hpp"

unsigned short PORT;
socks_record_info socks_record;
socks_server_info socks_server;

int main(int argc, char *argv[])
{
        int sockfd, clientfd, optval = 1;

        if (argc != 2)
        {
                fprintf(stderr, "Usage: ./http_server <port>");
                return 0;
        }

        PORT = (unsigned short)atoi(argv[1]);

        set_server_sockfd(&sockfd);
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        clean_zombie();

        while (1)
        {
                clientfd = get_client_fd(sockfd);
                if (clientfd > 0)
                {
                        switch (fork())
                        {
                        case -1:
                                perror("fork");
                                break;
                        case 0:
                                close(sockfd);
                                client_handler(clientfd);
                                close(clientfd);
                                break;
                        default:
                                close(clientfd);
                                break;
                        }
                }
        }
        close(sockfd);
        return 0;
}

static void clean_zombie()
{
        struct sigaction sa;
        sa.sa_handler = sigchld_handler,
        sa.sa_flags = SA_RESTART;

        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
                perror("sigaction");
                exit(EXIT_FAILURE);
        }
}

static void sigchld_handler(int s)
{
        while (waitpid(-1, NULL, WNOHANG) > 0)
                ;
}

static void client_handler(int browser_socket)
{
        int dst_socket, n, bind_fd;
        char buffer[BUFFER_SIZE] = {0};
        uint8_t socks4_reply[8] = {0};
        uint8_t socks4_request[BUFFER_SIZE] = {0};

        memset(&socks_server, 0, sizeof(socks_server));
        
        if ((n = read(browser_socket, socks4_request, BUFFER_SIZE)) < 0)
        {
                perror("read");
                exit(1);
        }
        
        // VN is the SOCKS protocol version number and should be 4.
        // VER: SOCKS version number, 0x04 for this version ===> SOCKS4
        // 		+----+----+----+----+----+----+----+----+----+----+....+----+
        // 		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
        // 		+----+----+----+----+----+----+----+----+----+----+....+----+
        //  # of bytes:	   1    1      2              4           variable       1


        if (socks4_request[0] != 0x04)
        {
                fprintf(stderr, "Usage: socks4\n");
                exit(0);
        }



        // unpacking the package buffer.
        parse_request(socks4_request);

        // Response packet from server
        // 	   +----+----+----+----+----+----+----+----+
        //         | VN | CD | DSTPORT |      DSTIP        |
	//         +----+----+----+----+----+----+----+----+
        //  # of bytes:	   1    1      2              4

        // VN is the version of the reply code and should be 0. CD is the result
        memcpy(socks4_reply, socks4_request, 8);
        socks4_reply[0] = 0;
        

        // Two operations are defined: CONNECT and BIND. ===> CD Column
        // CMD: command code:
        // 0x01 = establish a TCP/IP stream connection
        // 0x02 = establish a TCP/IP port binding

        if (socks4_request[1] == 0x01)
        {
                socks_record.command = "CONNECT";
                // connect mode.
                if ((dst_socket = tcp_connect()) < 0 || !fire_wall(socks4_request))
                
                //if ((dst_socket = tcp_connect()) < 0)
                
                {       
                        printf("socket = %d\n", dst_socket);
                         
                        if(fire_wall(socks4_request))
                            printf("Pass The Wall\n");
                        else
                            printf("Blocked by the Wall\n");
                        socks_record.reply = "Reject";
                        print_verbose();
                        socks4_reply[1] = 0x5B;
                        write(browser_socket, socks4_reply, 8);
                        close(dst_socket);
                        close(browser_socket);
                        exit(0);
                }
                else
                {
                        socks_record.reply = "Accept";
                        print_verbose();
                        socks4_reply[1] = 0x5A;
                        write(browser_socket, socks4_reply, 8);
                }
        }
        else if (socks4_request[1] == 0x02)
        {
                socks_record.command = "BIND";
                // bind mode.
                if (!fire_wall(socks4_request))
                {
                        socks_record.reply = "Reject";
                        print_verbose();
                        socks4_reply[1] = 0x5B;
                        close(browser_socket);
                        exit(0);
                }

                else
                {
                        socks_record.reply = "Accept";
                        print_verbose();
                        socks4_reply[0] = 0x00;
                        socks4_reply[1] = 0x5A;
                        dst_socket = do_bindmode(browser_socket, socks4_reply);
                }
        }

        // establishment success.
        // generate the reply package back to browser .
        do_transfer(browser_socket, dst_socket);
}

static void parse_request(uint8_t *request)
{
        /*Sock 4A*/
        if( !request[4] && !request[5] && !request[6] && request[7])
        {       
                 // byte 1 - 3 is port
                socks_server.cd_value = request[1];
                socks_server.dst_port = request[2] * 256 + request[3];
                socks_record.D_PORT = socks_server.dst_port;

                int domain_start_index;
                for(domain_start_index = 8 ; request[domain_start_index] ;domain_start_index++);
                string domain_name;
                for(int i = domain_start_index + 1 ; request[i] ; i++){ domain_name += (char)(request[i]); }

                struct addrinfo hints, *servinfo, *p;
                int rv;
                if ((rv = getaddrinfo(domain_name.c_str(), to_string(socks_server.dst_port).c_str(), &hints, &servinfo)) != 0) {
                        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                        exit(1);
                }
                
                char ip[INET_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET, (struct sockaddr_in*) &(servinfo->ai_addr), ip, INET_ADDRSTRLEN);
                socks_record.D_IP = string(ip);                
                socks_server.dst_ip = ((struct sockaddr_in*) servinfo->ai_addr)->sin_addr.s_addr;
                freeaddrinfo(servinfo);
                
                request[4] = (socks_server.dst_ip & 0x000000ff);
                request[5] = (socks_server.dst_ip & 0x0000ff00) >> 8;
                request[6] = (socks_server.dst_ip & 0x00ff0000) >> 16;
                request[7] = (socks_server.dst_ip & 0xff000000) >> 24;


        }

        /*Sock 4*/
        else{
                // byte 1 - 3 is port
                socks_server.cd_value = request[1];
                socks_server.dst_port = request[2] * 256 + request[3];
                socks_record.D_PORT = socks_server.dst_port;

                // byte 4 - 8 is IP - Address
                socks_record.D_IP = to_string(request[4]) + "." + to_string(request[5]) + "." + to_string(request[6]) + "." + to_string(request[7]);
                socks_server.dst_ip = request[7] << 24 | request[6] << 16 | request[5] << 8 | request[4];
        }
}


static void set_server_sockfd(int *listen_fd)
{
        int client_fd, rv;
        int yes = 1;
        struct addrinfo hints, *res, *p;
        char port[20] = {0};

        sprintf(port, "%hu", PORT);

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0)
        {
                fprintf(stderr, "selectserver :%s\n", gai_strerror(rv));
                exit(EXIT_FAILURE);
        }

        for (p = res; p; p = p->ai_next)
        {
                if ((*listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
                {
                        continue;
                }

                setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

                if (bind(*listen_fd, p->ai_addr, p->ai_addrlen) < 0)
                {
                        close(*listen_fd);
                        continue;
                }

                break;
        }

        if (p == NULL)
        {
                fprintf(stderr, "selectserver : failed to bind\n");
                exit(EXIT_FAILURE);
        }

        freeaddrinfo(res);

        if (listen(*listen_fd, 10) == -1)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }
}

static int get_client_fd(int sockfd)
{
        // struct sockaddr_storage remoteaddr;
        struct sockaddr_in remote_addr;
        int client_fd;
        socklen_t addrlen;
        char _s_ip[100] = {0};

        memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;

        addrlen = sizeof(remote_addr);
        client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &addrlen);
        inet_ntop(AF_INET, &remote_addr.sin_addr.s_addr, _s_ip, sizeof(remote_addr));

        socks_record.S_IP = string(_s_ip);
        socks_record.S_PORT = remote_addr.sin_port;
            
        if (client_fd == -1)
        {
                return -1;
        }
        else
        {
                return client_fd;
        }
}

static int tcp_connect()
{
        int dst_socket, n;
        struct sockaddr_in serv;

        dst_socket = socket(AF_INET, SOCK_STREAM, 0);
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = socks_server.dst_ip;
        serv.sin_port = htons(socks_server.dst_port);

        if (connect(dst_socket, (struct sockaddr *)&serv, sizeof(serv)) < 0)
        {
                cerr << "refuse connection" << endl;
                return -1;
        }
        return dst_socket;
}

static int fire_wall(uint8_t *req)
{
        ifstream configfile("socks.conf");
        string line;
        int permit = 0;
        
        if (configfile.is_open())
        {
                while (getline(configfile, line))
                {
                        vector<string> rs, ip;
                        boost::split( rs, line, boost::is_any_of( " " ), boost::token_compress_on );
                        boost::split( ip, rs[2], boost::is_any_of( "." ), boost::token_compress_on );
                        
                        
                        if ( (req[1] == 0x01 && rs[1] == "b") || (req[1] == 0x02 && rs[1] == "c")){continue;}
                        int star_num = count(ip.begin(), ip.end(), "*");
                        

                        switch(star_num){
                                case 4: 
                                        return 1;

                                case 3: 
                                        if( (uint8_t)stoi(ip[0]) == req[4]){
                                            return 1;
                                        }
                                        break;

                                case 2:
                                        if( (uint8_t)stoi(ip[0]) == req[4] && 
                                            (uint8_t)stoi(ip[1]) == req[5]   ) 
                                        {
                                            return 1;
                                        }
                                        break;
                                case 1:
                                        if( (uint8_t)stoi(ip[0]) == req[4] && 
                                            (uint8_t)stoi(ip[1]) == req[5] &&
                                            (uint8_t)stoi(ip[2]) == req[6]    )
                                            {   
                                                return 1;
                                            }
                                        break;
                                case 0:

                                        if( (uint8_t)stoi(ip[0]) == req[4] && 
                                            (uint8_t)stoi(ip[1]) == req[5] &&
                                            (uint8_t)stoi(ip[2]) == req[6] &&   
                                            (uint8_t)stoi(ip[3]) == req[7]
                                        ){
                                            return 1;
                                        }
                                        break;
                        }

                        
                }
                configfile.close();
        }

        else
            cout << "Unable to open socks.conf file\n";
        return 0;
}

static void do_transfer(int source_socket, int dest_socket)
{
        int len, fdmax, i, n;
        char buffer[BUFFER_SIZE] = {0};
        fd_set rfd, afd;

        fdmax = (source_socket > dest_socket) ? source_socket : dest_socket;

        FD_ZERO(&rfd);
        FD_ZERO(&afd);
        FD_SET(dest_socket, &afd);
        FD_SET(source_socket, &afd);

        while (1)
        {
                rfd = afd;

                if ((n = select(fdmax + 1, &rfd, NULL, NULL, NULL)) < 0)
                {
                        if (errno == EBADF)
                        {
                                exit(0);
                        }
                        else
                        {
                                continue;
                        }
                }

                for (i = 1; i <= fdmax; i++)
                {
                        memset(buffer, 0, sizeof(buffer));

                        if (i == source_socket && FD_ISSET(i, &rfd))
                        {
                                // read from browser , write into web.
                                len = read(i, buffer, sizeof(buffer));

                                if (!len)
                                {
                                        FD_CLR(i, &afd);
                                        close(source_socket);
                                        close(dest_socket);
                                        break;
                                }
                                else
                                {
                                        // printf("upload : %s\n",buffer);
                                        write(dest_socket, buffer, len);
                                }
                        }

                        else if (i == dest_socket && FD_ISSET(i, &rfd))
                        {
                                len = read(i, buffer, sizeof(buffer));

                                if (!len)
                                {
                                        FD_CLR(i, &afd);
                                        close(source_socket);
                                        close(dest_socket);
                                        break;
                                }

                                else
                                {
                                        write(source_socket, buffer, len);
                                }
                        }
                }
        }
}

static void print_verbose()
{
        printf("-----------------------\n");
        printf("<S_IP>: %s\n", socks_record.S_IP.data());
        printf("<S_PORT>: %hu\n", socks_record.S_PORT);
        printf("<D_IP>: %s\n", socks_record.D_IP.data());
        printf("<D_PORT>: %hu\n", socks_record.D_PORT);
        printf("<Command>: %s\n", socks_record.command.data());
        printf("<Reply>: %s\n", socks_record.reply.data());
        printf("-----------------------\n");
}

static int do_bindmode(int source_socket, uint8_t *socks4_reply)
{
        int dest_socket, addrlen, bind_socket;
        struct sockaddr_in dest_addr, sa;
        socklen_t len;

        memset(&dest_addr, 0, sizeof(dest_addr));

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_port = htons(INADDR_ANY);

        addrlen = sizeof(dest_addr);
        len = sizeof(dest_addr);

        bind_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (bind(bind_socket, (struct sockaddr *)&dest_addr, addrlen) < 0)
        {
                perror("bind");
                return -1;
        }

        if (getsockname(bind_socket, (struct sockaddr *)&sa, (socklen_t *)&len) < 0)
        {
                perror("getsockname");
                return -1;
        }

        if (listen(bind_socket, 10) < 0)
        {
                perror("listen");
                return -1;
        }

        socks4_reply[1] = 0x5A;
        socks4_reply[2] = (uint8_t)(ntohs(sa.sin_port) / 256);
        socks4_reply[3] = (uint8_t)(ntohs(sa.sin_port) % 256);
        for (int i = 4; i < 8; ++i)
                socks4_reply[i] = 0x00;

        memset(&sa, 0, sizeof(sa));

        write(source_socket, socks4_reply, 8);

        if ((dest_socket = accept(bind_socket, (struct sockaddr *)&sa, &len)) < 0)
        {
                perror("accept");
                return -1;
        }
        write(source_socket, socks4_reply, 8);
        // send again.

        return dest_socket;
}
