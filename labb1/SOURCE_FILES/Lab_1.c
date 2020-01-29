#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)


void * client_thread(char * name){
    mqd_t mq;
    MQconnect(&mq, name);
    struct mq_attr attr;

    if(mq_getattr(mq, &attr) == -1)
        handle_error("mq_getattr");
    
    void *buffer = malloc(attr.mq_msgsize);
    while(strncmp(buffer, "END", 3) != 0){
        sleep(1);
        int nr = MQread(mq, &buffer);
        printf("Bytes read: %i\n", nr);
        //printf("Received msg: %s\n\n", (char*)buffer);
        printPlanet(buffer);
    }
    return NULL;
}

int main(int argc, char*argv[]){
    pthread_t client;
    mqd_t message_queue;

    if(argc != 2){
        fprintf(stderr, "Usage: %s <mq-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    MQcreate(&message_queue, argv[1]);

    pthread_create(&client, NULL, client_thread, argv[1]);
    planet_type *buffer = (planet_type*)malloc(sizeof(planet_type));
    char *placeholder;
    while(strncmp(buffer, "END", 3) != 0){
        printf("Enter text to send here:\n");
        scanf("%s", placeholder);
        getRandomPlanet(buffer);
        //printf("You entered: \"%s\"\n", buffer);
        MQwrite(message_queue, buffer);
    }

    pthread_join(client, NULL);
    return 0;
}