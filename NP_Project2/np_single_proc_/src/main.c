#include <stdlib.h>
#include "server.h"
#include "shell.h"

int main(int argc, char **argv)
{
        if(argc < 2){
                fprintf(stderr, "usage: ./npserver <port>\n");
                exit(EXIT_FAILURE);
        }

        server(argv[1]);
        return EXIT_SUCCESS;
}