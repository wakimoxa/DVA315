#include "wrapper.h"

#define MAX_SIZE 1024

//Big note! If you think the arguments makes no sense, you are allowed to change them, as long as the basic functionality is kept
//In case you run in to blocking issues with reading from the queue, the option O_NONBLOCK is a hot tip

int MQcreate (mqd_t * mq, char * name)
{
 	//Should create a new messagequeue, use mq as reference pointer so you can reach the handle from anywhere
	//Should return 1 on success and 0 on fail
    *mq = mq_open(name, O_CREAT);
    if(*mq == NULL){
        return 0;
    }
    return 1;

}
int MQconnect (mqd_t * mq, char * name)
{
    /* Connects to an existing mailslot for writing Uses mq as reference pointer, so that you can reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/
    *mq = mq_open(name,  O_RDWR);
    if(*mq == NULL){
        return 0;
    }
    return 1;
}

void * MQread (mqd_t mq, void ** buffer)
{

    /* Read a msg from a mailslot, return nr Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* should return number of bytes read*/
    return mq_receive(mq, *buffer , MAX_SIZE, NULL);
}

int MQwrite (mqd_t mq, void * sendBuffer)
{
    /* Write a msg to a mailslot, return nr Uses mq as reference pointer, so that you can 	     reach the handle from anywhere*/
    /* should return number of bytes read         */
    if(mq_send(mq, sendBuffer, sizeof(sendBuffer), NULL) == 0)
        return 1;
    return 0;
}

int MQclose(mqd_t * mq, char * name)
{
    /* close a mailslot, returning whatever the service call returns Uses mq as reference pointer, so that you can
    reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/
    return mq_close(mq);
}



