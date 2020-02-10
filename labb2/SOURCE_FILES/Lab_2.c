typedef int buffer_item;
#define BUFFER_SIZE 5

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define DINING 1


//----------------------------------------PRODUCER CONSUMER SEGMENT----------------------------------------------
#define RAND_DIVISOR 100000000
#define TRUE 1

pthread_mutex_t mutex;

sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE]={1,2,3,4,5};
int counter;
int ret;
int count=6;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */
int remove_item(buffer_item *item);
int insert_item(buffer_item item);

void initializeData() {

    /* Create the mutex lock */
    pthread_mutex_init(&mutex, NULL);

    /* Create the full semaphore and initialize to 0 */
    ret = sem_init(&full, 0, 0);

    /* Create the empty semaphore and initialize to BUFFER_SIZE */
    ret = sem_init(&empty, 0, BUFFER_SIZE);

    /* Get the default attributes */
    pthread_attr_init(&attr);

    /* init buffer */
    counter = 0;
}

/* Producer Thread */
void *producer(void *param) {
    buffer_item item;

    printf("Producer created!\n");
    fflush(stdout);
    while(TRUE) {
        /* sleep for a random period of time */
        int rNum = rand() / RAND_DIVISOR;
        sleep(rNum);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        item = count++;
        if(insert_item(item)) {
            fprintf(stderr, " Producer %ld report error condition\n", (long) param);
	        fflush(stdout);
        }
        else {
            printf("producer %ld produced %d\n", (long) param, item);
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
	fflush(stdout);
    }
}

/* Consumer Thread */
void *consumer(void *param) {
    buffer_item item;

    while(TRUE) {
        /* sleep for a random period of time */
        int rNum = rand() / RAND_DIVISOR;
        sleep(rNum);
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        if(remove_item(&item)) {
            fprintf(stderr, "Consumer %ld report error condition\n",(long) param);
	    fflush(stdout);
        }
        else {
            printf("consumer %ld consumed %d\n", (long) param, item);
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
}

/* Add an item to the buffer */
int insert_item(buffer_item item) {
    /* When the buffer is not full add the item
    and increment the counter*/
    if(counter < BUFFER_SIZE) {
        buffer[counter] = item;//critical
        counter++;//critical

        return 0;
    }
    else { /* Error the buffer is full */
        return -1;
    }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
    /* When the buffer is not empty remove the item
     and decrement the counter */
    if(counter > 0) {
        *item = buffer[(counter-1)];//critical
        counter--;//critical
        return 0;
    }
    else { /* Error buffer empty */
        return -1;
    }
}
//----------------------------------------PRODUCER CONSUMER SEGMENT----------------------------------------------
//----------------------------------------DINING PHILOSOPHERS VARIABLES------------------------------------------
pthread_t philosophers[5];
//int forks[] = {1, 1, 1, 1, 1};
pthread_mutex_t forks[5];
sem_t max_dining;

int counter_array[5];


//----------------------------------------DINING PHILOSOPHERS SEGMENT--------------------------------------------
void pickup(int fork_left, int fork_right){
    pthread_mutex_lock(&forks[fork_left]);
    usleep(10000);
    pthread_mutex_lock(&forks[fork_right]);
    usleep(10000);
}

void putdown(int fork_left, int fork_right){
    pthread_mutex_unlock(&forks[fork_left]);
    pthread_mutex_unlock(&forks[fork_right]);
}


void * philosopher_function(void *index){
    counter_array[(int) index] = 0;
    printf("Started philosopher %i\n", (int) index);
    sleep(2);
    while(1){
        usleep(10000);//Thinking
        printf("Philosopher %i is hungry\n", (int) index);
        sem_wait(&max_dining);//max 4 philosophers can eat at the same time
        pickup((int) index, ((int) index + 1) % 5);
        printf("Philosopher %i is picked up forks\n", (int) index);
        usleep(10000);//Eating
        printf("Philosopher %i has eaten\n", (int) index);
        putdown((int) index, ((int) index + 1) % 5);
        sem_post(&max_dining);
        printf("Philosopher %i puts down forks\n", (int) index);
        counter_array[(int) index]++;
    }
}

void intHandler(int test){
    for(int i = 0; i < 5; i++){
        printf("Philosopher %i has eaten %i times total\n", i, counter_array[i]);
    }
}

void dining_philosophers()
{
    int i;

    signal(SIGINT, intHandler);

    sem_init(&max_dining, 0, 4);

    printf("Started DINING PHILOSOPHERS!\n\n");
    for(i = 0; i < 5; i++){
        pthread_mutex_init(&forks[i], NULL);
    }

    for(i = 0; i < 5; i++){
        pthread_create(&philosophers[i], NULL, philosopher_function, (void*)i);
        //printf("Started philosopher %i\n", i);
    }
    pause();
    //printf("This is error!\n");
}
//----------------------------------------DINING PHILOSOPHERS SEGMENT--------------------------------------------
int main(int argc, char *argv[]) {

    long i;
    void * status;
    /* Verify the correct number of arguments were passed in */
    //if(argc != 4) {
    //    fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
    //}

    int mainSleepTime = 30;//atoi(argv[1]); /* Time in seconds for main to sleep */
    int numProd = 10; //atoi(argv[2]); /* Number of producer threads */
    int numCons = 3; //atoi(argv[3]); /* Number of consumer threads */

    if (DINING == 1)
    {
    	//start dining philosophers
        dining_philosophers();
    }
    else //Start producer consumer
    {
    	initializeData();

    	/* Create the producer threads */
    	for(i = 0; i < numProd; i++)
    	{
    	    pthread_create(&tid,&attr,producer,(void*) i);
    	}

    	/* Create the consumer threads */
    	for(i = 0; i < numCons; i++)
    	{
    	    pthread_create(&tid,&attr,consumer, (void*) i);
    	}
    	pthread_join(tid, &status);
    	/* Sleep for the specified amount of time in milliseconds */
    	sleep(mainSleepTime);


    	/* Exit the program */
    	printf("Exit the program\n");
    }


    exit(0);
}
