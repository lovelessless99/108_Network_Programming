#include "main.h"

#include <sys/ipc.h>
#include <sys/shm.h>

void test(client** user_list)
{
        client* user1 = create_client(1, "127.0.0.1", "8080");
        insert_client(user_list, &user1);


        client* user2 = create_client(2, "127.0.0.1", "8081");
        insert_client(user_list, &user2);

        client* user3 = create_client(3, "127.0.0.1", "8082");
        insert_client(user_list, &user3);

        client* user4 = create_client(4, "127.0.0.1", "8083");
        insert_client(user_list, &user4);

        client* user5 = create_client(5, "127.0.0.1", "8084");
        insert_client(user_list, &user5);

        print_each_client(*user_list);

        delete_client(user_list, 1);
        delete_client(user_list, 3);


        print_each_client(*user_list);

        client* user6 = create_client(6, "127.0.0.1", "80800");
        insert_client(user_list, &user6);


        delete_client(user_list, 5);
        delete_client(user_list, 1);

        print_each_client(*user_list);
        
        delete_client(user_list, 2);
        delete_client(user_list, 4);

        print_each_client(*user_list);

        client* user7 = create_client(7, "127.0.0.1", "80800");
        insert_client(user_list, &user7);

        delete_client(user_list, 6);
        delete_client(user_list, 7);
        print_each_client(*user_list);

}

void test2()
{
        Tube* tube_list = NULL;

        Tube* tube1 = create_tube(1, 2);
        push_tube(&tube_list, &tube1);

        Tube* tube2 = create_tube(1, 3);
        push_tube(&tube_list, &tube2);

        Tube* tube3 = create_tube(1, 4);
        push_tube(&tube_list, &tube3);

        Tube* tube4 = create_tube(1, 5);
        push_tube(&tube_list, &tube4);

        Tube* tube5 = create_tube(3, 2);
        push_tube(&tube_list, &tube5);

        Tube* tube6 = create_tube(2, 3);
        push_tube(&tube_list, &tube6);

        printf("Before Delete\n");
        printList(tube_list);
        delete_all_id_tube(&tube_list, 2);

        printf("After Delete 2\n");
        printList(tube_list);
        delete_all_id_tube(&tube_list, 1);

        printf("After Delete 1\n");
        printList(tube_list);
        delete_all_id_tube(&tube_list, 3);

        printf("After Delete 3\n\n");
        
        printList(tube_list);
}


void test_mem()
{
        int i;
        
        
        char pathname[30] ;
        strcpy(pathname,"./tmp") ;

        key_t key = ftok(pathname, 3);
        int shm_id = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0666 );
        client *client_list = (client*)shmat(shm_id, NULL, 0);
        // test(&client_list);
        if(key < 0)
	{
		perror("ftok");
		return;
	}
	if(shm_id < 0)
	{
		perror("shmget");
		return;
	}
        shmdt(client_list);
}

int main(int argc, char **argv)
{
        test_mem();
        // test2();
        // test();
        // if(argc < 2){
        //         fprintf(stderr, "usage: ./npserver <port>\n");
        //         exit(EXIT_FAILURE);
        // }

        // server(argv[1]);
        // return EXIT_SUCCESS;
}