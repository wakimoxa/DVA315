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

void getPlanet(planet_type * pt, char* name, double mass, double Px, double Py, double Vx, double Vy, int life);



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

int main(int argc, char*argv[])
{

	//setup
	printf("Hello i'm a client with pid: %i\n\n", getpid());
	mqd_t mq_server;
	mqd_t mq_client;

	char* client_name = (char*)malloc(sizeof(char)*20);
	sprintf(client_name, "/mq_%i", getpid());
	MQcreate(&mq_client, client_name);

	char* server_name = "/server_mq";
	MQconnect(&mq_server, server_name);

	planet_type *buffer = (planet_type*)malloc(sizeof(planet_type));


	//loop
	while(1){
		char* name = (char*)malloc(sizeof(char)*20);;
		double mass, Px, Py, Vx, Vy;
		int life;

		/*
		printf("Enter planet name: ");
		scanf("%s", name);
		printf("Enter planet mass: ");
		scanf("%lf", &mass);
		printf("Enter planet Pos: ");
		scanf("%lf %lf", &Px, &Py);
		printf("Enter planet Vel: ");
		scanf("%lf %lf", &Vx, &Vy);
		printf("Enter planet life: ");
		scanf("%lf", &life);
		*/
		scanf("%s", name);
		mass = 30;
		Px = Py = 200;
		Vx = Vy = 0.01;
		life = 10000000;
		getPlanet(buffer, name, mass, Px, Py, Vx, Vy, life);

		MQwrite(mq_server, buffer);

	}


	
	return EXIT_SUCCESS;
}
