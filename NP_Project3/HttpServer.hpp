#ifndef _HTTPSERVER_HPP
#define _HTTPSERVER_HPP

#include <boost/asio.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <cstdlib>
#include <netdb.h>
#include <string>
#include <memory>
#include <map>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
boost::asio::io_service global_io_service;

static void filter(string&);
static void clean_zombie();
static void sigchld_handler(int);

class echo_session: public enable_shared_from_this<echo_session>
{
        public:
            echo_session(tcp::socket socket);
            void start_service();

        private:
            void do_service();
            void set_env_var();
            void get_http_data();
            void parse_http_header();
            
        private:
            tcp::socket _socket;
            vector<string> env_names;
            map<string, string> env;
            int max_length;
            vector<char> message;
            string cgi_name;

};

class tcp_server
{
	public:
		tcp_server(short port);

	private:
		void do_accept();

	private:
			tcp::socket _socket;
			tcp::acceptor _acceptor;
			tcp::endpoint _endpoint;
};

#endif