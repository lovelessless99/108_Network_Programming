#include "client.h"
#include "shamem.h"

void init_client(int clientfd, pid_t clientpid)
{
	struct sockaddr_in addr;
	//struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));

    socklen_t len = sizeof(addr);
    
	if (getpeername(clientfd, (struct sockaddr *)&addr, &len) < 0)
	{
		perror("getpeername");
	}

	int client_idx = get_available_client();	
	memset(&client_info[client_idx], 0, sizeof(client_info[client_idx]));
	
	client_info[client_idx].commandTable = NULL;
		
	sprintf(client_info[client_idx].name, "%s", "(no name)");
	sprintf(client_info[client_idx].path, "%s", "bin:.");

    inet_ntop(addr.sin_family, &(addr.sin_addr), client_info[client_idx].ip_address, INET_ADDRSTRLEN);
	
    client_info[client_idx].client_fd = clientfd;
	client_info[client_idx].cnt_line = 0;
	client_info[client_idx].enable = 1;
	client_info[client_idx].pid = clientpid;
	client_info[client_idx].id = client_idx;
	client_info[client_idx].port = (uint16_t)ntohs(addr.sin_port);
	
}

int get_available_client()
{
	for (int target = 1; target <= MAX_CLIENTS; ++target)
	{
		if (client_info[target].client_fd == 0)
		{
			return target;
		}
	}
	return -1;
}

client *get_client(int clientfd)
{
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (client_info[i].client_fd == clientfd)
		{
			return &client_info[i];
		}
	}
	return NULL;
}

client *get_client_by_id(int target_id)
{
	client *ptr = NULL;
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (client_info[i].id == target_id)
		{
			ptr = &client_info[i];
		}
	}
	return ptr;
}

client *get_client_by_pid(pid_t target_pid)
{
	client *ptr = NULL;
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (client_info[i].pid == target_pid)
		{
			ptr = &client_info[i];
		}
	}
	return ptr;
}