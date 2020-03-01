#include "main.h"


void test()
{
        client* user_list = NULL;
        client* user1 = create_client(1, "127.0.0.1", "8080");
        insert_client(&user_list, &user1);


        client* user2 = create_client(2, "127.0.0.1", "8081");
        insert_client(&user_list, &user2);

        client* user3 = create_client(3, "127.0.0.1", "8082");
        insert_client(&user_list, &user3);

        client* user4 = create_client(4, "127.0.0.1", "8083");
        insert_client(&user_list, &user4);

        client* user5 = create_client(5, "127.0.0.1", "8084");
        insert_client(&user_list, &user5);

        print_each_client(user_list);

        delete_client(&user_list, 1);
        delete_client(&user_list, 3);


        print_each_client(user_list);

        client* user6 = create_client(6, "127.0.0.1", "80800");
        insert_client(&user_list, &user6);


        delete_client(&user_list, 5);
        delete_client(&user_list, 1);

        print_each_client(user_list);
        
        delete_client(&user_list, 2);
        delete_client(&user_list, 4);

        print_each_client(user_list);

        client* user7 = create_client(7, "127.0.0.1", "80800");
        insert_client(&user_list, &user7);

        delete_client(&user_list, 6);
        delete_client(&user_list, 7);
        print_each_client(user_list);

}
int main(int argc, char **argv)
{
        test();
        // if(argc < 2){
        //         fprintf(stderr, "usage: ./npserver <port>\n");
        //         exit(EXIT_FAILURE);
        // }

        // server(argv[1]);
        // return EXIT_SUCCESS;
}