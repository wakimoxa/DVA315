#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)

struct msg_args{
    char * name;
    char * data;
};


void * send_msg(void* input){
    sleep(2);
    printf("Sending msg\n");
    mqd_t mq;
    char * data = ((struct msg_args*)input)->data;
    char * name = ((struct msg_args*)input)->name;
    mq = mq_open(name, O_WRONLY);
    if(mq == (mqd_t) -1)
        handle_error("mq_open");
    if(mq_send(mq, data, strlen(data) + 1, NULL) == -1)
        handle_error("mq_send");
    printf("Msg sent\n");
    sleep(4);
    return NULL;
}


static void received_msg(union sigval sv){
    struct mq_attr attr;
    ssize_t nr;
    void* buff;
    mqd_t mqdes = *((mqd_t *) sv.sival_ptr);

    printf("Receiver started\n");

    if(mq_getattr(mqdes, &attr) == -1)
        handle_error("mq_getattr");
    
        buff = malloc(attr.mq_msgsize);
    if(buff == NULL)
        handle_error("malloc");
    
    printf("Exec mq_receive\n");
    nr = mq_receive(mqdes, buff, attr.mq_msgsize, NULL);
    if(nr = -1)
        handle_error("mq_receive");
    
    printf("Read %zd bytes from MQ\n", nr);
    printf("%s", (char *)buff);
    free(buff);
    exit(EXIT_SUCCESS);
}

int main(int argc, char*argv[]){
    mqd_t mqdes;
    struct sigevent sev;
    if(argc != 2){
        fprintf(stderr, "Usage: %s <mq-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mqdes = mq_open(argv[1], O_RDONLY | O_CREAT, 0666, NULL);
    if(mqdes == (mqd_t) -1)
        handle_error("mq_open");

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = received_msg;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = &mqdes;
    if(mq_notify(mqdes, &sev) == -1)
    handle_error("mq_notify");
    pthread_t send_thread;
    struct msg_args *msg = (struct msg_args*)malloc(sizeof(struct msg_args));
    char data[] = "Hello, this is msg\n";
    msg->data = data;
    msg->name = argv[1];

    if(pthread_create(&send_thread, NULL, send_msg, (void *)msg)) {
			printf("Error creating thread\n");
			return 1;
		}
    pthread_join(send_thread, NULL);
    pause();
    return 0;
}