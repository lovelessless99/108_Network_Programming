#include "npshell.h"

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr, "usage: ./npserver <port>\n");
        exit(EXIT_FAILURE);
    }
	server(argv[1]);
    return EXIT_SUCCESS;
}

