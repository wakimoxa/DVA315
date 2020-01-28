#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>


void * client_thread(char * name){
    mqd_t mq;
    MQconnect(&mq, name);
    char *buffer;
    MQread(mq, &buffer);
    pause();
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
    char *buffer;
    while(strncmp(buffer, "END", 3) != 0){
        printf("Enter text to send here:\n");
        scanf("%s", buffer);
        //printf("You entered: \"%s\"\n", buffer);
        MQwrite(message_queue, buffer);
    }

    pthread_join(client, NULL);
    return 0;
}