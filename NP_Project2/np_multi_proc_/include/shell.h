#ifndef _SEHLL_H_
#define _SHELL_H_

typedef struct CommandTable CommandTable;

typedef void (*launch_t)(void);
typedef void (*printenv_t)(char*);
typedef void (*execute_t)(CommandTable*);


struct myShell{
    int (*launch)(int, pid_t);
    void (*printenv)(char*);
    void (*execute)(CommandTable*);
    void (*execute_pipeline)(CommandTable*);
};

void init_shell(struct myShell**);
#endif