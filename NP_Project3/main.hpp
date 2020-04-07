#ifndef _MAIN_HPP
#define _MAIN_HPP

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/asio.hpp>

#include <string.h>
#include <iostream>
#include <direct.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <errno.h>

#define SERVER_SIZE 5
#define ITEM_NUM 3

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

boost::asio::io_service global_io_service;
boost::asio::io_service io_service_for_client;

#define MAX_LENGTH 4096
#define SERVER_SIZE 5

typedef struct server_info server_info;

struct server_info
{
	int server_index;
	int enable;
	string domain_name;
	string port;
	string filename;
} ;


class echo_session : public enable_shared_from_this<echo_session>
{
        public:
                echo_session(tcp::socket socket);
                void start_service();
                tcp::socket _socket;

        private:
                void send_cgi_web();
                void process_console_cgi();
                void print_html_web();
                void parse_query(string query);
                void do_write(const char *_buffer);
                void do_service();
                void get_http_data();
                void parse_http_header();
                void filter(string &sentence);

        private:
                
                vector<char> message;
                vector<string> env_names;
                map<string, string> env;
                string cgi_name;
};


class tcp_client
{
        public:
                tcp_client();
                void get_start(server_info& _server, echo_session& http_obj);

        private:
                void network_entity(string&);
                void do_resolve(server_info& _server, echo_session& http_obj);
                void do_connect(server_info& _server, tcp::resolver::iterator& it, echo_session& http_obj);
                void do_read(server_info& _server, echo_session& http_obj);
                void do_write(const char *buffer);
                void output_shell(char *_message, server_info& _server, echo_session& http_obj);
                void send_command(server_info& _server, echo_session& http_obj);
        
        public:
	        ifstream fin;

        private:
	        tcp::resolver _resolver;
                tcp::socket _socket;
                char message[MAX_LENGTH] = { 0 };

};


class tcp_server
{
        public:
	        tcp_server(unsigned short port);
        private:
	        void do_accept();

        private:
                tcp::socket _socket;
                tcp::acceptor _acceptor;
                tcp::endpoint _endpoint;

};


#endif