#include "main.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#define tube_name "tube"
#define tube_projid 3
#define client_name "client"
#define client_projid 10



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




int main(int argc, char **argv)
{
        
        Tube** tube_list = NULL;
        client** user_list = NULL;

        int fd_1 = open(client_name, O_CREAT | O_RDWR );
        int fd_2 = open(tube_name, O_CREAT | O_RDWR );
        

        key_t key_user = ftok(client_name, client_projid);
        key_t key_tube = ftok(tube_name, tube_projid);
        
        printf("key_tube: %d, key_user: %d\n", key_tube, key_user);
        int shmid_tube = shmget(key_tube, __getpagesize() , IPC_CREAT | 0666 );
        int shmid_user = shmget(key_user, __getpagesize() , IPC_CREAT | 0666 );
        
        printf("shmid_tube: %d, shmid_user: %d\n", shmid_tube, shmid_user);
        if ( shmid_tube == -1 ) { perror("shmid_tube"); }
        if ( shmid_user == -1 ) { perror("shmid_user"); }

        


        tube_list = shmat(shmid_tube, NULL, 0);
        user_list = shmat(shmid_user, NULL, 0);

        

        client* user1 = create_client(1, "127.0.0.1", "8080");
        insert_client(user_list, &user1);


        client* user2 = create_client(2, "127.0.0.1", "8081");
        insert_client(user_list, &user2);

        client* user3 = create_client(3, "127.0.0.1", "8082");
        insert_client(user_list, &user3);

        client* user4 = create_client(4, "127.0.0.1", "8083");
        insert_client(user_list, &user4);

        client* user5 = create_client(5, "127.0.0.1", "8084");
        insertclient(user_list, &user5);

        print_each_client(*user_list);

        delete_client(user_list, 1);
        delete_client(user_list, 3);


        print_each_client(*user_list);

        client* user6 = create_client(6, "127.0.0.1", "80800");
        insert_client(user_list, &user6);


        delete_client(user_list, 5);
        delete_client(user_list, 1);

        print_each_client(*user_list);

        // int pid = fork();
        // if (pid == 0)
        // {
        //         printf("In child process\n");
                

        //         key_t key_user = ftok("/dev/null", client_projid);
        //         key_t key_tube = ftok("/dev/null", tube_projid);
                

        //         int shmid_tube = shmget(key_tube, sizeof(Tube**), IPC_CREAT | IPC_EXCL | 0660);
        //         int shmid_user = shmget(key_user, sizeof(client**), IPC_CREAT | IPC_EXCL  | 0660);
                
        //         tube_list = shmat(shmid_tube, NULL, 0);
        //         user_list = shmat(shmid_user, NULL, 0);
                
        //         client* user7 = create_client(7, "127.0.0.1", "80801111");
        //         insert_client(user_list, &user7);

        //         client* user8 = create_client(8, "127.0.0.1", "80811111");
        //         insert_client(user_list, &user8);

        //         client* user9 = create_client(9, "127.0.0.1", "80821111");
        //         insert_client(user_list, &user9);

        //         client* user10 = create_client(10, "127.0.0.1", "80831111");
        //         insert_client(user_list, &user10);

        //         client* user11 = create_client(11, "127.0.0.1", "80841111");
        //         insert_client(user_list, &user11);

        //         print_each_client(*user_list);
        // }
        // test2();
        // test();
        // if(argc < 2){
        //         fprintf(stderr, "usage: ./npserver <port>\n");
        //         exit(EXIT_FAILURE);
        // }
        
        shmdt(tube_list);
        shmdt(user_list);

        unlink(client_name);
        unlink(tube_name);
        // server(argv[1]);
        // return EXIT_SUCCESS;
}