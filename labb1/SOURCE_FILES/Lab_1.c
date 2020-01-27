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


void *loop(int *thread_x){
	for(int i = 0; i < 10; i++){
		printf("Hello Moon!\n");
		usleep(1000000);
	}
	return NULL;
}

int main(int ac, char * argv)
{
	pthread_t my_thread;
	int x = 0;
	while(1){
		for(int i = 0; i < 10; i++){
			printf("Hello world!\n");
			usleep(1000000);
		}
		if(pthread_create(&my_thread, NULL, &loop, &x)) {
			printf("Error creating thread\n");
			return 1;
		}
	}
	return 0;

}
