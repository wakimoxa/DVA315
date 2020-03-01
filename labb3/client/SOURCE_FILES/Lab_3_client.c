/*
 ============================================================================
 Name        : Lab_3_client.c
 Author      : Jakob Danielsson
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "wrapper.h"
#include <unistd.h>
#include <signal.h>


#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)

void getPlanet(planet_type * pt, char* name, double mass, double Px, double Py, double Vx, double Vy, int life);

mqd_t mq_server;
mqd_t mq_client;



void getPlanet(planet_type * pt, char* name, double mass, double Px, double Py, double Vx, double Vy, int life){
	pt->life = life;
	pt->mass = mass;
	strcpy(pt->name, name);
	sprintf(pt->pid, "%i", getpid());
	pt->sx = Px;
	pt->sy = Py;
	pt->vx = Vx;
	pt->vy = Vy;
	pt->next = NULL;
}

void * mq_reader(void*args){
    char * name = (char*)args;
	//MQclose(&message_queue, name);
    MQcreate(&mq_client, name);

    struct mq_attr attr;

    if(mq_getattr(mq_client, &attr) == -1)
        handle_error("mq_getattr");
    while(1){
        //sleep(1);
        void *buffer = malloc(attr.mq_msgsize);
        int nr = MQread(mq_client, &buffer);
        printf("Server received %i bytes.\n", nr);
        //printf("Received msg: %s\n\n", (char*)buffer);
        char* message = (char*) buffer;
        if(strncmp(message, "END", 3) != 0)
        {
            printf("%s\n", message);
        }
        else
            break;
    }
    return NULL;
}

pthread_t mq_reader_thread;

void intHandler(int test)
{
	MQclose(&mq_client, "hehe xd, this is useless");
	MQclose(&mq_server, "hehe xd, this is useless");
}



int main(int argc, char*argv[])
{

	//setup
	printf("Hello i'm a client with pid: %i\n\n", getpid());

	char* client_name = (char*)malloc(sizeof(char)*20);
	sprintf(client_name, "/mq_%i", getpid());

	char* server_name = "/server_mq";
	MQconnect(&mq_server, server_name);

	pthread_create(&mq_reader_thread, NULL, mq_reader, (void*)client_name);

    signal(SIGINT, intHandler);

	//loop
	while(1){
		planet_type *buffer = (planet_type*)malloc(sizeof(planet_type));

		char* name = (char*)malloc(sizeof(char)*20);;
		double mass, Px, Py, Vx, Vy;
		int life;

		printf("Enter planet name: ");
		scanf("%s", name);
		printf("Enter planet mass: ");
		scanf("%lf", &mass);
		printf("Enter planet Pos: ");
		scanf("%lf %lf", &Px, &Py);
		printf("Enter planet Vel: ");
		scanf("%lf %lf", &Vx, &Vy);
		printf("Enter planet life: ");
		scanf("%i", &life);

		getPlanet(buffer, name, mass, Px, Py, Vx, Vy, life);

		do{
			MQwrite(mq_server, buffer);
			usleep(500000);
		}while(strncmp(name, "qwe", 3) == 0);

		free(buffer);
	}


	
	return EXIT_SUCCESS;
}
