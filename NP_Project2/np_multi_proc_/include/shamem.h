#ifndef _SHAMEM_H_
#define _SHAMEM_H_


#define _CLIENT "/shm_client"
#define _MESSAGE "/shm_message"
#define _USER_PIPE "/shm_user_pipe"
#define _LOGCNT "/shm_logcnt"

#include "global.h"
#include "socket.h"

extern int shm_client_info_fd, shm_message_fd, shm_userpipe_fd, shm_login_fd;
extern int   (*user_pipe)[MAX_CLIENTS+1][1];

extern int    *login_cnt;
extern char   *share_message;
extern client *client_info;


void init_share_mem();
void init_client_info();
void free_share_mem();
void *get_shared_memory(int fd, size_t total_size);
#endif