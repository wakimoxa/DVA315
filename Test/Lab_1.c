/*
 ============================================================================
 Name        : Lab_1.c
 Author      : Jakob Danielsson
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <pthread.h>
#include <unistd.h>


pthread_mutex_t lock;

void *hello_moon(int *thread_x){
	pthread_mutex_lock(&lock);

	for(int i = 0; i < 10; i++){
		printf("Hello Moon!\n");
		usleep(1000000);
	}
	pthread_mutex_unlock(&lock);
	return NULL;
}

void *hello_world(int *thread_x){
	pthread_mutex_lock(&lock);
	for(int i = 0; i < 10; i++){
		printf("Hello World!\n");
		usleep(1000000);
	}
	pthread_mutex_unlock(&lock);
	return NULL;
}


int main(int ac, char * argv)
{
	pthread_t world_thread;
	pthread_t moon_thread;
	
	int x = 0;
	while(1){
		if(pthread_create(&world_thread, NULL, &hello_world, &x)) {
			printf("Error creating thread\n");
			return 1;
		}
		if(pthread_create(&moon_thread, NULL, &hello_moon, &x)) {
			printf("Error creating thread\n");
			return 1;
		}
		pthread_join(world_thread, NULL);
		pthread_join(moon_thread, NULL);
		//pthread_mutex_destroy(&lock);
	}
	return 0;

}
