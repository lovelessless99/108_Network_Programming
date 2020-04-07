#include "HttpServer.hpp"


echo_session::echo_session(tcp::socket socket): _socket(move(socket)),
                                                max_length(4096),
                                                message(max_length)
                                                
{
        env_names = {"SERVER_PROTOCOL","REQUEST_METHOD", "REQUEST_URI", "QUERY_STRING", "HTTP_HOST", 
                     "SERVER_ADDR"    ,"SERVER_PORT"   , "REMOTE_ADDR", "REMOTE_PORT"};
        
        for(string env_name : env_names) { env[env_name] = "";}
}

void echo_session::start_service()
{
        tcp::endpoint remote_endpoint   = _socket.remote_endpoint();
	tcp::endpoint local_endpoint	= _socket.local_endpoint();

        env["REMOTE_ADDR"] = remote_endpoint.address().to_string();
        env["REMOTE_PORT"] = to_string(remote_endpoint.port());
        env["SERVER_ADDR"] = local_endpoint.address().to_string();

        
        get_http_data();
}

void echo_session::do_service()
{
        auto self(shared_from_this());
        char status_code[] = "HTTP/1.1 200 OK\n";
        clean_zombie();
        _socket.async_send(
        boost::asio::buffer(status_code, strlen(status_code)), [this, self] (const boost::system::error_code ec, size_t write_length)
        {
                int socketfd = _socket.native_handle();					
                if (!ec)
                {
                        pid_t pid;
                        global_io_service.notify_fork(boost::asio::io_service::fork_prepare);

                        cgi_name = string(".", 1) + cgi_name;
                        // cout << cgi_name << endl;

                        if ((pid = fork()) == -1)
                        {
                                perror("fork");
                                exit(1);
                        }
                        
                        else if (pid == 0)
                        {
                                dup2(socketfd, STDOUT_FILENO);
                                global_io_service.notify_fork(boost::asio::io_service::fork_child);
                                _socket.close();
                                
                                if(execlp(cgi_name.c_str(), cgi_name.c_str(), NULL) < 0 )
                                {
                                        cout << "error!" << endl;
                                }
                                exit(1);
                        }
                        else
                        {
                                global_io_service.notify_fork(boost::asio::io_service::fork_parent);
                                _socket.close();
                                wait(NULL);
                        }

                }
        }
);
}

void echo_session::set_env_var()
{
        for(string env_name : env_names) { filter(env[env_name]); setenv(env_name.c_str(), env[env_name].c_str(), 1); }
}

void echo_session::get_http_data()
{
        auto self(shared_from_this());
        _socket.async_read_some(
                boost::asio::buffer (message, max_length),
                [this, self] (const boost::system::error_code ec, size_t length)
                {       
                        
                        if (!ec)
                        {
                                parse_http_header();	
                                set_env_var();
                                do_service();
                        }
                }
        );
}



void echo_session::parse_http_header()
{
        vector<string> header_info;

        stringstream ss;
        copy(message.begin(), message.end(),ostream_iterator<char>(ss,""));

        string item;
        while (getline(ss, item)){ header_info.push_back(item);}
        header_info.pop_back();

        for(string line : header_info)
        {       
                string header;
               
                if(line == header_info[0]){
                        vector<string> temp;
                        boost::split( temp, line, boost::is_any_of( " " ), boost::token_compress_on );

                        env["REQUEST_METHOD"] = temp[0];
                        env["SERVER_PROTOCOL"] = temp[2];

                        if(temp[1].find("?") != string::npos)
                        {       
                                vector<string> tmp;
                                boost::split( tmp, temp[1], boost::is_any_of("?"), boost::token_compress_on );
                                env["REQUEST_URI"] = cgi_name = tmp[0];
                                env["QUERY_STRING"] = tmp[1];
                        
                        }

                        else{
                                env["REQUEST_URI"] = cgi_name = temp[1];
                        }        
                }

                if (line.find("Host") != string::npos) {
                        string::size_type index = line.find(": ", 0);
                        if( (index  = line.find(": ", 0)) != string::npos) { header = line.substr(index+1); }
                        if( (index  = header.find(":" , 0)) != string::npos){
                                env["HTTP_HOST"]   = header.substr(0, index);
                                env["SERVER_PORT"] = header.substr(index+1); 
                        }
                        break;
                }
        }

        // for(string envname : env_names){
        //         cout << envname << " " << env[envname] << endl;
        //  }
        // cout << "cgi_name "<< cgi_name << endl;
}


static void filter(string &sentence)
{
        if (!sentence.empty() && sentence[sentence.size() - 1] == '\r')
                sentence.erase(sentence.size() - 1);
}

static void clean_zombie(){
        struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa , NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

static void sigchld_handler(int){
        while (waitpid(-1, NULL, WNOHANG) > 0);
}

tcp_server::tcp_server(short port): _socket(global_io_service),
			            _acceptor(global_io_service),
			            _endpoint(tcp::v4(), port)
{
        _acceptor.open(_endpoint.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.bind(_endpoint);
        _acceptor.listen();
        do_accept();
}


void tcp_server::do_accept()
{
        _acceptor.async_accept(_socket, [this](const boost::system::error_code ec)
                {
                        if (!ec)
                        {
                                // success
                                make_shared<echo_session>(move(_socket))->start_service(); 
                                // _socket.close();
                                
                        }
                                do_accept();
                                
        });

}


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./http_server <port>");
		return 0;
	}

	unsigned short port = (unsigned short)atoi(argv[1]);


	tcp_server http_server(port);
	global_io_service.run();

	return 0;
}
