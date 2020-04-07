#include "Console.hpp"
server_info server[6];
socks_info socks;

boost::asio::io_service global_io_service;



tcp_client::tcp_client(): _socket (global_io_service),
                          _resolver (global_io_service),
                          socks_resolver(global_io_service)
{
}


void tcp_client::do_resolve(server_info& _server)
{
        tcp::resolver::query query(socks.host, socks.port);
        _resolver.async_resolve (query, [this, &_server] (const boost::system::error_code ec, tcp::resolver::iterator it)
        {
            if (!ec) { do_connect (_server, it); }
            else{ cerr << "resolve error!" << endl; }
        });
}


void tcp_client::do_connect (server_info& _server, tcp::resolver::iterator& it)
{
        tcp::endpoint _endpoint = it->endpoint();
        _socket.async_connect (_endpoint, [this, &_server] (boost::system::error_code ec)
        {       
                if (!ec){ 
                    uint8_t request[500] = {0};
                    uint8_t reply[8] = {0};
                    
                    vector<string> ip;

                    tcp::resolver::query _query(_server.domain_name, _server.port);
                    string ss = socks_resolver.resolve(_query)->endpoint().address().to_string();
                    
                    boost::split(ip, ss, boost::is_any_of("."), boost::token_compress_on );

                    uint32_t port = (uint32_t) stoi(_server.port);
                    request[0] = 0x04;
                    request[1] = 0x01;
                    request[2] = port / 256;
                    request[3] = port % 256;

                    /* Sock4 */
                    //for (int i = 4; i <= 7; i++) { request[i] = (uint8_t)stoi(ip[i-4]);}

                    /* Sock- 4A */
                    for (int i = 4; i < 7; i++) { request[i] = 0;}
                    request[7] = 1;
                    
                    request[8] = 'H';
                    request[9] = 'i';
                    request[10] = 0;
                    
                    string domain_name = "nplinux3.cs.nctu.edu.tw";
                    for(int i = 0 ; i < domain_name.length(); i++) request[11+i] = domain_name[i];
                    request[11 + domain_name.length()] = 0;


                    _socket.send(boost::asio::buffer(request, 4096));
                    _socket.read_some(boost::asio::buffer(reply, 8));

                    if (reply[1] == 0x5A) { do_read (_server); }
                }
                else{ cerr << "Connect Failed!" << endl; }
        });

        return ;
}

void tcp_client::do_read (server_info& _server)
{
        _socket.async_read_some (
            boost::asio::buffer (message, MAX_LENGTH),
            [this, &_server] (const boost::system::error_code ec, size_t length)
        {
            if (!ec)
            {
                output_shell (message, _server);
                if (boost::algorithm::contains(string(message), "% "))
                {
                    memset (message, 0, sizeof(message));
                    send_command (_server);
                }    
                memset (message, 0, sizeof(message));
                do_read (_server);
            }

            else{ cerr << "read failed!" << endl; }
        });
        return;
}

void tcp_client::do_write (char *buffer, size_t length)
{
        _socket.async_write_some (
        boost::asio::buffer (buffer, length), [&] (const boost::system::error_code ec, size_t write_length) {});
        return;
}


void tcp_client::send_command (server_info& _server)
{
        char buffer[4096] = {0};
        
        if (!fin.getline(buffer, 4096))
        {
            fin.close();
            return;
        }

        size_t n = strlen(buffer);
        string content(buffer);

        boost::replace_all(content, "\r", "&NewLine;");
        boost::replace_all(content, "\n", "&NewLine;");
        
        network_entity(content);
        printf("<script>document.getElementById(\'s%d\').innerHTML += \'<b>%s</b>&NewLine;\'</script>", _server.server_index, content.c_str());
         
        buffer[n] = '\n';
        do_write (buffer, n + 1);
}

static void parse_query(string query)
{
    vector<string> info;
    boost::split(info, query, boost::is_any_of("&"));

    int server_index = 1;
    for(int i = 0 ; i < (int)info.size()-2; i+=3, server_index++)
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

    int host_index = info[15].find("=") + 1;
    int sock_port_index = info[16].find("=") + 1;

    socks.host = info[15].substr(host_index);
    socks.port = info[16].substr(sock_port_index);

}

static void print_html_web()
{
    cout << "Content-type: text/html" << endl << endl;
    cout << "<meta charset=\"UTF-8\">";
    cout << "<title>NP Project 3 Console</title>";
    cout << "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\"/>";
    cout << "<link href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\" rel=\"stylesheet\"/><link rel=\"icon\" type=\"image/png\" href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"/>";

    cout << "<style>* { font-family: 'Source Code Pro', monospace; font-size: 1rem !important}body {background-color: #212529;}pre {color: #cccccc;}b{color: #ffffff;}</style>";

    printf("</head>");
    printf("<body>");
    printf("<table class=\"table table-dark table-bordered\">");
    printf("<thead>");
    printf("<tr>");

    for (int i = 1; i <= SERVER_SIZE; ++i)
    {
    
        if (server[i].enable)
        {
            printf("<th scope=\"col\">%s:%s</th>", server[i].domain_name.c_str(), server[i].port.c_str());
        }
    }

   printf("</tr>");
   printf("</thead>");
   printf("<tbody>");
   printf("<tr>");

    int server_index = 1;
    for (int i = 1; i <= SERVER_SIZE; ++i)
    {
        if (server[i].enable)
        {
            printf("<td><pre id=\"s%d\" class=\"mb-0\"></pre></td>", server_index);
            server_index++;
        }
    }
    printf("</tr>");
    printf("</tbody>");
    printf("</table>");
    printf("</body>");
    printf("</html>");

}

static void output_shell (char *message, server_info& _server)
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
        printf ("<script>document.getElementById(\'s%d\').innerHTML += \'%s\';</script>\n", _server.server_index, temp.c_str());
    }
}

static void network_entity(string &buffer)
{
    const string original[] = {"<", ">", " ", "\"", "\'", "\n", "\r"};
    const string replaces[] = {"&lt;", "&gt;", "&nbsp;", "&quot;", "&apos;", "&NewLine;", "&NewLine;"};

    for(int i = 0 ; i < 7 ; i++)
        boost::algorithm::replace_all(buffer, original[i], replaces[i]);
}




int main (void)
{

    int server_index = 1;
    parse_query (string(getenv ("QUERY_STRING")));
    
    print_html_web();
    tcp_client client[6];
    try
    {
        for (int i = 1; i <= SERVER_SIZE; ++i)
        {
            if (server[i].enable)
            {
                server[i].server_index = server_index;
                chdir("test_case");
                client[i].fin.open(server[i].filename, std::ifstream::in);
                chdir("../");
                client[i].do_resolve(server[i]);
                server_index++;
            }
        }
    }
    catch (std::exception &e)
    {
        cerr << "Exception: " << e.what() << endl;
    }
    global_io_service.run();

    return 0;
}


