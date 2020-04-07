#ifndef _CONSOLE_HPP
#define _CONSOLE_HPP

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>


#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <string>
#include <cstdlib>
#include <memory>

#include <iostream>
#include <fstream>

#define MAX_LENGTH 4096
#define SERVER_SIZE 5
#define ITEM_NUM 3

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;


typedef struct
{
    int server_index;
    int enable;
    string domain_name;
    string port;
    string filename;
} server_info;

typedef struct
{
    string host;
    string port;
    int enable;

}socks_info;

static void parse_query (string);
static void print_html_web();
static void output_shell (char *, server_info &);
static void network_entity(string&);


// setting the total number of connection.

class tcp_client
{
        public:
        tcp_client();
        void do_resolve (server_info& _server);
        void do_connect (server_info& _server, tcp::resolver::iterator& it);
        void do_read (server_info& _server);
        void do_write (char *buffer, size_t length); /* write data to server. */
        void send_command (server_info& _server); /* read file and write commands to server */
 
        public:
        ifstream fin;
        
        private:
        tcp::socket _socket;
        tcp::resolver _resolver;
        tcp::resolver socks_resolver;
        char message[MAX_LENGTH];
};

#endif
