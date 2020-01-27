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
	while(1){
		printf("Hello Moon! %i\n", *thread_x);
		usleep(200000);
	}
	return NULL;
}

int main(int ac, char * argv)
{
	pthread_t my_thread;
	int x = 0;

	for(int i = 0; i < 10; i++){
		if(pthread_create(&my_thread, NULL, loop, &x)) {
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
		printf("Hello world!\n");
		usleep(1000000);
	}
}
