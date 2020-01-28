#include "wrapper.h"
#include <signal.h>

#define MAX_SIZE 1024

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)

//Big note! If you think the arguments makes no sense, you are allowed to change them, as long as the basic functionality is kept
//In case you run in to blocking issues with reading from the queue, the option O_NONBLOCK is a hot tip

static void received_msg(union sigval sv){
    struct mq_attr attr;
    ssize_t nr;
    void* buff;
    mqd_t mqdes = *((mqd_t *) sv.sival_ptr);


    if(mq_getattr(mqdes, &attr) == -1)
            handle_error("mq_getattr");
        buff = malloc(attr.mq_msgsize);
    if(buff == NULL)
        handle_error("malloc");
        
    printf("Receiver started\n");
    while(strncmp(buff, "END", 3) != 0){
        nr = mq_receive(mqdes, buff, (attr.mq_msgsize), 0);
        if(nr == -1)
            handle_error("mq_receive");
        
        printf("Read %zd bytes from MQ\n", nr);
        printf("%s\n", (char *)buff);
    }
    free(buff);
    exit(EXIT_SUCCESS);
    return;
}

int MQcreate (mqd_t * mq, char * name)
{
 	//Should create a new messagequeue, use mq as reference pointer so you can reach the handle from anywhere
	//Should return 1 on success and 0 on fail
    printf("Creating %s\n", name);
    *mq = mq_open(name, O_WRONLY | O_CREAT, 0666, NULL);
    if(*mq == (mqd_t) -1)
        handle_error("mq_open");
    return 1;

}
int MQconnect (mqd_t * mq, char * name)
{
    /* Connects to an existing mailslot for writing Uses mq as reference pointer, so that you can reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/
    printf("Connecting to %s\n", name);
    *mq = mq_open(name, O_RDONLY);
    if(*mq == (mqd_t) -1)
        handle_error("mq_open");

    return 1;
}

int MQread (mqd_t mq, void ** buffer)
{
    /* Read a msg from a mailslot, return nr Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* should return number of bytes read*/
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = received_msg;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = &mq;

    if(mq_notify(mq, &sev) == -1)
        handle_error("mq_notify");
    pause();
}

int MQwrite (mqd_t mq, void * data)
{
    /* Write a msg to a mailslot, return nr Uses mq as reference pointer, so that you can 	     reach the handle from anywhere*/
    /* should return number of bytes read         */
    if(mq_send(mq, data, strlen(data) + 1, 0) == -1)
        handle_error("mq_send");
    return 1;
}

int MQclose(mqd_t * mq, char * name)
{
    /* close a mailslot, returning whatever the service call returns Uses mq as reference pointer, so that you can
    reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/
if(mq_close(*mq) == -1)
    handle_error("mq_close");

return 1;
}

