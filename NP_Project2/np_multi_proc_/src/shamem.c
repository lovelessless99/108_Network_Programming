#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "shamem.h"
#include "client.h"

int shm_client_info_fd, shm_message_fd, shm_userpipe_fd, shm_login_fd;

int   (*user_pipe)[MAX_CLIENTS+1][1];
int    *login_cnt;
char   *share_message;
client *client_info;


void init_share_mem()
{
    int flag = O_RDWR | O_CREAT;

    shm_unlink(_LOGCNT);
	shm_unlink(_CLIENT);
	shm_unlink(_MESSAGE);
	shm_unlink(_USER_PIPE);

	shm_client_info_fd = shm_open(_CLIENT, flag, 0666);
	shm_message_fd = shm_open(_MESSAGE, flag, 0666);
	shm_userpipe_fd = shm_open(_USER_PIPE, flag, 0666);
	shm_login_fd = shm_open(_LOGCNT, flag, 0666);

	ftruncate(shm_client_info_fd, sizeof(client) * (MAX_CLIENTS + 1));
	ftruncate(shm_message_fd, sizeof(char) * BUFFSIZE);
	ftruncate(shm_userpipe_fd, sizeof(int) * (BUFFSIZE + 1) * (BUFFSIZE + 1) * 1);	
	ftruncate(shm_login_fd, sizeof(int));
}

void init_client_info()
{
    int user_pipe_size = sizeof(int) * (MAX_CLIENTS + 1) * (MAX_CLIENTS + 1) * 1;
    int message_size = sizeof(char) * BUFFSIZE;
    int client_info_size =  sizeof(client) * (MAX_CLIENTS + 1);

    user_pipe = (int(*)[MAX_CLIENTS + 1][1]) get_shared_memory(shm_userpipe_fd, user_pipe_size);
	share_message = (char *) get_shared_memory(shm_message_fd, message_size);
	client_info = (client *) get_shared_memory(shm_client_info_fd, client_info_size);
	login_cnt = (int *) get_shared_memory(shm_login_fd, sizeof(int));

	memset(user_pipe, 0, sizeof(int) * (MAX_CLIENTS + 1) * (MAX_CLIENTS + 1) * 1);
	memset(share_message, 0, sizeof(char) * BUFFSIZE);
	memset(client_info, 0, sizeof(client) * (MAX_CLIENTS + 1));

    login_cnt = 0;
}

void free_share_mem()
{
    shm_unlink(_CLIENT);
	shm_unlink(_MESSAGE);
	shm_unlink(_USER_PIPE);
	shm_unlink(_LOGCNT);
	close(shm_client_info_fd);
	close(shm_message_fd);
	close(shm_userpipe_fd);
	close(shm_login_fd);
    exit(EXIT_FAILURE);
}

void *get_shared_memory(int fd, size_t total_size)
{
	return mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	
}