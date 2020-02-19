#include "wrapper.h"
#define MAX_SIZE 1024

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)

//Big note! If you think the arguments makes no sense, you are allowed to change them, as long as the basic functionality is kept
//In case you run in to blocking issues with reading from the queue, the option O_NONBLOCK is a hot tip

int MQcreate (mqd_t * mq, char * name)
{
 	//Should create a new messagequeue, use mq as reference pointer so you can reach the handle from anywhere
	//Should return 1 on success and 0 on fail
    //printf("Creating %s\n", name);
    *mq = mq_open(name, O_WRONLY | O_CREAT, 0666, NULL);
    if(*mq == (mqd_t) -1)
        handle_error("mq_open");
    return 1;
}

int MQconnect (mqd_t * mq, char * name)
{
    /* Connects to an existing mailslot for writing Uses mq as reference pointer, so that you can reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/
    //printf("Connecting to %s\n", name);
    *mq = mq_open(name, O_RDONLY);
    if(*mq == (mqd_t) -1)
        handle_error("mq_open");

    return 1;
}

int MQread (mqd_t mq, void ** buffer)
{
    /* Read a msg from a mailslot, return nr Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* should return number of bytes read*/
    struct mq_attr attr;
    ssize_t nr;
    if(mq_getattr(mq, &attr) == -1)
        handle_error("mq_getattr");
    
    nr = mq_receive(mq, *buffer, (attr.mq_msgsize), 0);
    if(nr == -1)
        handle_error("mq_receive");
    return (int)nr;

}

int MQwrite (mqd_t mq, void * data)
{
    /* Write a msg to a mailslot, return nr Uses mq as reference pointer, so that you can 	     reach the handle from anywhere*/
    /* should return number of bytes read         */
    if(mq_send(mq, data, sizeof(planet_type) + 1, 0) == -1)
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

void getRandomPlanet(planet_type * pt, char name[]){
    srand(time(NULL));
    
    pt->life = rand() % 10000;
    pt->mass = rand() % 20000;
    strcpy(pt->name, name);
    sprintf(pt->pid, "%i", rand() % 100);
    pt->sx = rand()  % 100;
    pt->sy = rand()  % 100;
    pt->vx = rand()  % 100;
    pt->vy = rand()  % 100;
}

void printPlanet(planet_type *pt){
    printf("Planet life: %i\n", pt->life);
    printf("Planet mass: %f\n", pt->mass);
    printf("Planet name: %s\n", pt->name);
    printf("Planet life: %s\n", pt->pid);
    printf("Planet coordinates: (%f, %f)\n", pt->sx, pt->sy);
    printf("Planet velocity: (%f, %f)\n", pt->vx, pt->vy);
}
