#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

#include "history.h"

void initialize(History**);
static void append(History** history, char*);
static void save(History**);


void initialize(History** history)
{
    (*history) = malloc(sizeof(history));
    (*history)->append = append;
    (*history)->save = save;
}

static void append(History** history, char* cmd)
{
    (*history)->history =  strdup(cmd);
}

static void save(History** history)
{
    char *home = getpwuid(getuid())->pw_dir;
    int mode = O_WRONLY | O_CREAT | O_APPEND;
    int permission = S_IRUSR | S_IWUSR;

    char *filename = "/.npshell_history";
    strcat(home, filename);
    int fd = open(home, mode, permission);
    strcat((*history)->history, "\n");
    write(fd, (*history)->history, strlen((*history)->history));
    close(fd);
    //free(home);
}