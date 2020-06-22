#ifndef _HISTORY_H_
#define _HISTORY_H_

typedef struct history History;
typedef void (*append_t)(History**, char*);
typedef void (*save_t)(History**);
typedef void (*init_t)(History**);

void initialize(History**);

struct history{
    char * history;
    append_t append;
    save_t save;
};
#endif