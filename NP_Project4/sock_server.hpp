#ifndef __SOCK_SERVER_HPP
#define __SOCK_SERVER_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <algorithm> 

using namespace std;
typedef struct
{
        unsigned short dst_port;
        unsigned short source_port;
        uint32_t dst_ip;
        uint32_t source_ip;
        uint8_t cd_value;
} socks_server_info;

typedef struct
{
        string S_IP;
        string D_IP;
        unsigned short S_PORT;
        unsigned short D_PORT;
        string command;
        string reply;
} socks_record_info;


#define BUFFER_SIZE 4096

static void clean_zombie();
static void sigchld_handler(int);

static void print_verbose();
static void client_handler(int);
static void parse_request(uint8_t *);
static void set_server_sockfd(int *);
static void do_transfer(int, int);
static int do_bindmode(int, uint8_t *);
static int fire_wall(uint8_t *);
static int tcp_connect();
static int get_client_fd(int);

#endif