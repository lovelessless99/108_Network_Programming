#include "main.hpp"

server_info server[6];

echo_session::echo_session(tcp::socket socket)
			: _socket(move(socket)),
			  message(MAX_LENGTH)
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

void echo_session::send_cgi_web()
{
	auto self(shared_from_this());

	string webPage( "Content-type: text/html\n\n<!DOCTYPE html><html lang = \"en\"><head><title>NP Project 3 Panel</title><link rel =\"stylesheet\" href =\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity =\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin =\"anonymous\"/><link href =\"https://fonts.googleapis.com/css?family=Source+Code+Pro\" rel =\"stylesheet\"/><link rel =\"icon\" type =\"image/png\" href =\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"/><style>* {\nfont - family: \'Source Code Pro\', monospace;}</style></head><body class = \"bg-secondary pt-5\"><form action =\"console.cgi\" method = \"GET\"><table class =\"table mx-auto bg-light\" style =\"width: inherit\"><thead class =\"thead-dark\"><tr><th scope =\"col\">#</th><th scope =\"col\">Host</th><th scope =\"col\">Port</th><th scope =\"col\">Input File</th></tr></thead><tbody>");

	string domain_name = "cs.nctu.edu.tw";
	vector<string> test_files = {"t1.txt", "t2.txt", "t3.txt", "t4.txt", "t5.txt", "t6.txt", "t7.txt", "t8.txt", "t9.txt", "t10.txt"};
	vector<string> hosts = {"nplinux1", "nplinux2", "nplinux3", "nplinux4", "nplinux5"};

	string test_case_menu = "<option></option>";
	string host_menu = "<option></option>";

	for(string test_file : test_files) { test_case_menu +=  "<option value=\"" + test_file + "\">" + test_file + "</option>";}
	for(string host: hosts) { host_menu += ("<option value=\"" + host + "." + domain_name + "\">" + host + "</option>");}


	for(int i = 0; i < (int)hosts.size(); i++)
	{ 
		webPage += "<tr>";
		webPage = webPage + "<th scope =\"row\" class =\"align-middle\">Session" + std::to_string(i + 1) + "</th>";
		webPage += "<td>";
		webPage += "<div class =\"input-group\">";
		webPage = webPage + "<select name =\"h" + std::to_string(i) + "\" class =\"custom-select\">";
		webPage = webPage + host_menu + "</select>";
		webPage += "<div class =\"input-group-append\"><span class =\"input-group-text\">.cs.nctu.edu.tw</span></div></div></td><td>";
		webPage = webPage + "<input name =\"p" + std::to_string(i) + "\" type =\"text\" class =\"form-control\" size =\"5\" />";
		webPage += "</td><td>";
		webPage = webPage + "<select name =\"f" + std::to_string(i) + "\" class =\"custom-select\">";
		webPage = webPage + test_case_menu + "</select></td></tr>";
	}

	webPage += "<tr><td colspan =\"3\"></td><td><button type=\"submit\" class = \"btn btn-info btn-block\">Run</button></td></tr></tbody></table></form></body></html>";
	do_write(webPage.c_str());
}

void echo_session::process_console_cgi()
{
	int server_index = 1;
	parse_query(env["QUERY_STRING"]);
	print_html_web();
	tcp_client client[SERVER_SIZE + 1];

	try
	{		
		for (int i = 1; i <= SERVER_SIZE; ++i)
		{
			if (server[i].enable)
			{
				server[i].server_index = server_index;
				server_index++;
				_chdir("test_case");
				client[i].fin.open(server[i].filename, std::ifstream::in);
				_chdir("../");
				client[i].get_start(server[i], *this);
			}
		}
		io_service_for_client.run();
	}
	catch (std::exception &e) { cerr << "Exception: " << e.what() << endl; }
}

void echo_session::print_html_web()
{
	string webPage("Content-type: text/html\n\n<meta charset=\"UTF-8\"><title>NP Project 3 Console</title><link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\"/><style>* { font-family: 'Source Code Pro', monospace; font-size: 1rem !important}body {background-color: #212529;}pre {color: #cccccc;}b{color: #ffffff;}</style></head><br><body><br><table class=\"table table-dark table-bordered\"><br><thead><br><tr><br>");
	for (int i = 1; i <= SERVER_SIZE; ++i) { if (server[i].enable) { webPage += "<th scope=\"col\">" + string(server[i].domain_name) + ":" + string(server[i].port) + "</th><br>";} }
	webPage += "</tr><br></thead><br><tbody><br><tr><br>";

	for (int i = 1, server_index = 1; i <= SERVER_SIZE; ++i){ if (server[i].enable){ webPage += "<td><pre id=\"s"+ std::to_string(server_index++) + "\" class=\"mb-0\"></pre></td><br>";} }
	webPage += "</tr><br></tbody><br></table><br></body><br></html>\n";
	do_write(webPage.c_str());
}

void echo_session::parse_query(string query)
{
	vector<string> info;
	boost::split(info, query, boost::is_any_of("&"));

	int server_index = 1;
	for(int i = 0 ; i < (int)info.size(); i+=3, server_index++)
	{
		if(info[i].back() == '=') break;
		server[server_index].enable = 1;

		int domain_index = info[i].find("=") + 1;
		server[server_index].domain_name = info[i].substr(domain_index);

		int port_index   = info[i+1].find("=") + 1;
		server[server_index].port = info[i+1].substr(port_index);

		int file_index   = info[i+2].find("=") + 1;
		server[server_index].filename = info[i+2].substr(file_index);    
	}
}

void echo_session::do_write(const char *_buffer)
{	
	_socket.async_send(
		boost::asio::buffer(_buffer, strlen(_buffer)), [this](const boost::system::error_code ec, size_t w_len)
		{
				if (!ec){}
				else { cout << "async_write err : " << ec << endl; }
	});
}


void echo_session::do_service()
{
	auto self(shared_from_this());
		char status_code[50] = "HTTP/1.1 200 OK\n";

		_socket.async_send(
			boost::asio::buffer(status_code, 50), [this, self](const boost::system::error_code ec, size_t write_length)
		{
			if (!ec)
			{	
				if(cgi_name.compare("/panel.cgi") == 0)  { send_cgi_web();}
				if(cgi_name.compare("/console.cgi") == 0){ process_console_cgi();}
			}
			else { cout << "async_send err : " << ec << endl; }
		});
}

void echo_session::get_http_data()
{
	auto self(shared_from_this());
		_socket.async_read_some(
		boost::asio::buffer(message, MAX_LENGTH),
		[this, self](const boost::system::error_code ec, size_t length)
		{
			if (!ec)
			{
				parse_http_header();
				do_service();
			}
			else { cout << "asyc_read : err : " << ec << endl; }
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

                        else{ env["REQUEST_URI"] = cgi_name = temp[1]; }        
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
}

void echo_session::filter(string &sentence)
{
	if (!sentence.empty() && sentence[sentence.size() - 1] == '\r')
                sentence.erase(sentence.size() - 1);
}



tcp_client::tcp_client():_resolver(io_service_for_client),
			 _socket(io_service_for_client) {}


void tcp_client::get_start(server_info& _server, echo_session& http_obj) { do_resolve(_server, http_obj);}

void tcp_client::network_entity(string &buffer)
{
	const string original[] = {"<", ">", " ", "\"", "\'", "\n", "\r"};
    	const string replaces[] = {"&lt;", "&gt;", "&nbsp;", "&quot;", "&apos;", "&NewLine;", "&NewLine;"};

    	for(int i = 0 ; i < 7 ; i++)
        	boost::algorithm::replace_all(buffer, original[i], replaces[i]);
}

void tcp_client::do_resolve(server_info& _server, echo_session& http_obj)
{
	tcp::resolver::query query(_server.domain_name, _server.port);
	_resolver.async_resolve(query, [&](const boost::system::error_code ec, tcp::resolver::iterator it)
	{
		if (!ec) { do_connect(_server, it, http_obj); }
		else { cout << "async resolve: " << ec << endl;}
	});
}

void tcp_client::do_connect(server_info& _server, tcp::resolver::iterator& it, echo_session& http_obj)
{
	tcp::endpoint _endpoint = it->endpoint();
	_socket.async_connect(_endpoint, [&](boost::system::error_code ec)
	{
		if (!ec) { do_read(_server, http_obj); }
		else { cout << "async connect:" << ec << endl; }
	});
}


void tcp_client::do_read(server_info& _server, echo_session& http_obj)
{
		_socket.async_read_some(
			boost::asio::buffer(message, MAX_LENGTH),
			[&](const boost::system::error_code ec, size_t length)
		{
			if (!ec)
			{
				output_shell(message, _server, http_obj);
				if (boost::algorithm::contains(string(message), "% "))
				{
					memset(message, 0, sizeof(message));
					send_command(_server, http_obj);
				}		
				memset(message, 0, sizeof(message));
				do_read(_server, http_obj);

			}
			else
			{
				cout << "Async read: " << ec << endl;
			}
		});
}


void tcp_client::output_shell(char *_message, server_info& _server, echo_session& http_obj)
{
	string content(message);
    	stringstream ss (content);
    	string item;

	while (getline (ss, item, '\n'))
	{
		if (item != "% ")
		item.append("&NewLine;");
		string temp(item);
		network_entity(temp);
		temp = "<script>document.getElementById(\'s" + std::to_string(_server.server_index) +"\').innerHTML += \'" + temp + "\';</script>\n";
		
		http_obj._socket.async_send(
		boost::asio::buffer(temp.c_str(), temp.length()), [this](const boost::system::error_code ec, size_t write_len)
		{
			if (!ec){ }
			else { cout << "output_shell async: " << ec << endl;}
		});
	}
}

void tcp_client::do_write(const char *buffer){ _socket.async_write_some(boost::asio::buffer(buffer, strlen(buffer)), [&](const boost::system::error_code ec, size_t write_length) {});}

void tcp_client::send_command(server_info& _server, echo_session& http_obj)
{
        string content, server_command;
	if (!getline(fin, content))
        {
            fin.close();
            return;
        }
		
	server_command = content + "\n";
	boost::replace_all(content, "\r", "&NewLine;");
        boost::replace_all(content, "\n", "&NewLine;");


        network_entity(content);
        content = boost::str(boost::format("<script>document.getElementById(\'s%d\').innerHTML += \'<b>%s</b>\'; </script>\n") % _server.server_index % content.c_str());
	http_obj._socket.async_send(
              boost::asio::buffer(content, content.length()),
              [this](boost::system::error_code ec, size_t length) {
                  if (!ec) {}
                  else { cerr << "send_command error: " << ec << endl;}
	});
	do_write (server_command.c_str());
}


tcp_server::tcp_server(unsigned short port): _socket(global_io_service),
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
			make_shared<echo_session>(move(_socket))->start_service();
			_socket.close();
		}
		do_accept();
	});
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./http_server <port>");
		return 0;
	}

	unsigned short port = (unsigned short)atoi(argv[1]);
	putenv("PATH=.;./test_case");
	try
	{
		tcp_server http_server(port);
		global_io_service.run();
	}
	catch (std::exception &e) { cerr << "Exception: " << e.what() << endl; }
	return 0;
}