#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)


void * client_function(char * name){
    mqd_t mq;
    MQconnect(&mq, name);
    struct mq_attr attr;

    if(mq_getattr(mq, &attr) == -1)
        handle_error("mq_getattr");
    
    void *buffer = malloc(attr.mq_msgsize);
    while(1){
        //sleep(1);
        int nr = MQread(mq, &buffer);
        printf("Bytes read: %i\n", nr);
        //printf("Received msg: %s\n\n", (char*)buffer);
        planet_type* pt = (planet_type*) buffer;
        if(strncmp(pt->name, "END", 3) != 0)
            printPlanet(buffer);
        else
            break;
    }
    return NULL;
}

void * server_function(char * name){
    mqd_t message_queue;
    MQcreate(&message_queue, name);
    planet_type *buffer = (planet_type*)malloc(sizeof(planet_type));
    char placeholder[100];
    while(strncmp(placeholder, "END", 3) != 0){
        //placeholder = NULL;
        printf("Enter text to send here: ");
        scanf("\n%[^\n]%*c", placeholder);
        if(placeholder){
            getRandomPlanet(buffer, placeholder);
            printf("You entered: \"%s\"\n", placeholder);
            MQwrite(message_queue, buffer);
        }
        else{
            sleep(1);
        }
    }
    return NULL;
}

int main(int argc, char*argv[]){
    pthread_t client;//Client thread
    pthread_t server;//Server thread

    if(argc != 2){ //If no argument is given -> terminate
        fprintf(stderr, "Usage: %s <mq-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char server_name[strlen(argv[1])];
    char client_name[strlen(argv[1])];

    strcpy(server_name, argv[1]);
    strcpy(client_name, argv[1]);

    pthread_create(&server, NULL, server_function, argv[1]);
    sleep(1);
    pthread_create(&client, NULL, client_function, argv[1]);

    pthread_join(server, NULL);
    pthread_join(client, NULL);
    return 0;
}