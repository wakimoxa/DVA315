/*
 ============================================================================
 Name        : OSsched.c
 Author      : Jakob Danielsson
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : RR scheduler simulator, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<time.h>

#define sched_RR 1
#define sched_SJF 2
#define sched_MQ 3

#define QUEUE_SIZE 10
int sched_type = sched_MQ;
int finished = 0;
int context_switch_program_exit = 0;
int context_switch = 0;
int OS_cycles = 0;
int context_switches = 0;

typedef struct taskprop{
    int deadline;		//Deadline of a task, not necesarry to use
    int period;			//Periodicity of a task, not necesarry to use if you dont want to create a periodic scheduler such as rate monotonic. If you want to do this, talk with Jakob first
    int release_time;		//The time when a task is supposed to start, i.e., released from waiting queue
    int priority;		//Priority of task, can be used for the multiple queues
    int ID;			//ID, to distinguish different tasks from eachother
    int quantum;		//How long the task has left to execute
    int queue_size;
    struct taskprop * next;
    struct taskprop * previous;
} task;

task * ready_queue = NULL;
task * waiting_queue = NULL;
task * exec_task;
task * idle_task;

task * high_priority;
task * medium_priority;
task * low_priority;


task tasks[QUEUE_SIZE]; //Queue
int activeTasks = 0;
int idleTasks = 0;

//Implementera insertfunktioner





//------------------Linked list functions------------------
void copy_task(task ** dest, task * src)		//Copies data of a task to another task
{
	task * temp = *dest;
	temp->ID = src->ID;
	temp->deadline = src->deadline;
	temp->release_time = src->release_time;
	temp->period = src->period;
	temp->priority = src->priority;
	temp->quantum = src->quantum;
	*dest = temp;
}



task* create(int ID, int deadline, int release_time, int period, int prio, int quantum, task* next)			//Creates a new task
{
    task* new_node = (task*)malloc(sizeof(task));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    new_node->ID = ID;
    new_node->deadline = deadline;
    new_node->release_time = release_time;
    new_node->period = period;
    new_node->priority = prio;
    new_node->quantum = quantum;
    new_node->next = next;
    return new_node;
}
task * push(task * head, task data)			//Appends a task to a list
{
    /* go to the last node */
    task *cursor = head;
    if (cursor == NULL)
    {
    	head =  create(data.ID, data.deadline, data.release_time, data.period, data.priority, data.quantum ,NULL);
    }
    else
    {
    	while(cursor->next != NULL)
    	{
    		cursor = cursor->next;
    	}
    /* create a new node */
    	task* new_node =  create(data.ID, data.deadline, data.release_time, data.period, data.priority, data.quantum ,NULL);
    	cursor->next = new_node;
    }
    return head;
}


task * pop(task * head)			//Pops the first element of a list
{
    if(head == NULL)
        return NULL;
    task *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}

task * remove_back(task* head)			//Removes the back of a list
{
    if(head == NULL)
        return NULL;

    task *cursor = head;
    task *back = NULL;
    while(cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }
    if(back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if(cursor == head)
        head = NULL;

    free(cursor);

    return head;
}

task* remove_front(task* head)
{
    if(head == NULL)
        return NULL;
    task *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}
task * remove_node (task* head,task* nd)
{
    /* if the node is the first node */
    if(nd == head)
    {
        head = pop(head);
        return head;
    }

    /* if the node is the last node */
    if(nd->next == NULL)
    {
        head = remove_back(head);
        return head;
    }

    /* if the node is in the middle */
    task* cursor = head;
    while(cursor != NULL)
    {
        if(cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if(cursor != NULL)
    {
    	task* tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}
task * first_to_last (task * head)
{
	if (head==NULL)
		return NULL;
	if (head->next == NULL)
		return head;

	task * new_front = head->next;
	task* cursor = new_front;
	while(cursor->next != NULL)
	{
		cursor = cursor->next;
	}
	cursor->next = head;
	head->next=NULL;
	return new_front;

}
//---my sorting functions-----
// Returns the last node of the list 
task *getTail(task *cur) 
{ 
    while (cur != NULL && cur->next != NULL) 
        cur = cur->next; 
    return cur; 
} 

// Partitions the list taking the last element as the pivot 
task *partition(task *head, task *end, 
                    task **newHead, task **newEnd) 
{ 
    task *pivot = end; 
    task *prev = NULL, *cur = head, *tail = pivot; 
  
    // During partition, both the head and end of the list might change 
    // which is updated in the newHead and newEnd variables 
    while (cur != pivot) 
    { 
        if (cur->quantum < pivot->quantum) 
        { 
            // First node that has a value less than the pivot - becomes 
            // the new head 
            if ((*newHead) == NULL) 
                (*newHead) = cur; 
  
            prev = cur;  
            cur = cur->next; 
        } 
        else // If cur node is greater than pivot 
        { 
            // Move cur node to next of tail, and change tail 
            if (prev) 
                prev->next = cur->next; 
            task *tmp = cur->next; 
            cur->next = NULL; 
            tail->next = cur; 
            tail = cur; 
            cur = tmp; 
        } 
    } 
  
    // If the pivot data is the smallest element in the current list, 
    // pivot becomes the head 
    if ((*newHead) == NULL) 
        (*newHead) = pivot; 
  
    // Update newEnd to the current last node 
    (*newEnd) = tail; 
  
    // Return the pivot node 
    return pivot; 
} 
  
  
//here the sorting happens exclusive of the end node 
task *quickSortRecur(task *head, task *end) 
{ 
    // base condition 
    if (!head || head == end) 
        return head; 
  
    task *newHead = NULL, *newEnd = NULL; 
  
    // Partition the list, newHead and newEnd will be updated 
    // by the partition function 
    task *pivot = partition(head, end, &newHead, &newEnd); 
  
    // If pivot is the smallest element - no need to recur for 
    // the left part. 
    if (newHead != pivot) 
    { 
        // Set the node before the pivot node as NULL 
        task *tmp = newHead; 
        while (tmp->next != pivot) 
            tmp = tmp->next; 
        tmp->next = NULL; 
  
        // Recur for the list before pivot 
        newHead = quickSortRecur(newHead, tmp); 
  
        // Change next of last node of the left half to pivot 
        tmp = getTail(newHead); 
        tmp->next = pivot; 
    } 
  
    // Recur for the list after the pivot element 
    pivot->next = quickSortRecur(pivot->next, newEnd); 
  
    return newHead; 
} 
  
// The main function for quick sort. This is a wrapper over recursive 
// function quickSortRecur() 
void quickSort(task **headRef) 
{ 
    (*headRef) = quickSortRecur(*headRef, getTail(*headRef)); 
    return; 
} 

//---my sorting functions-----

//------------------Reads a taskset from file and inserts them to ready queue------------------
void readTaskset_n(char * filepath)
{
	FILE *reads;											//File handle
	char * sp = "/home/filip/C_programs/DVA315/labb4/tasks.txt";//File path
    reads=fopen(sp, "r");									//Open file
    task * data_struct = malloc(sizeof(task));				//Allocate data
    if (reads==NULL) {										//If reading fails, return
      	perror("Error");
       	return;
   	}
    else
	{
	    while(!feof(reads))									//Iterate through file
		{
			fscanf(reads,"%d %d %d %d %d %d\n", &data_struct->deadline, &data_struct->period, &data_struct->release_time, &data_struct->priority, &data_struct->ID, &data_struct->quantum);
			waiting_queue=push(waiting_queue, *data_struct);	//Read file and push it to the waiting queue
        }
    }
    free(data_struct);										//Free the struct
}

//------------------Wake up task - moves tasks from waiting queue to ready queue------------------
void OS_wakeup_n()
{
	if (waiting_queue == NULL)			//If waiting queue is empty, return
	{
		return;
	}
	task * temp = waiting_queue;		//Handle to head of waiting queue
	while (temp != NULL)				//Iterate through waiting queue
	{
		if (OS_cycles >= temp->release_time)		//If the OS clock is greater or equal to the release time of a task, push it to the ready queue
		{
			ready_queue=push(ready_queue, *temp);				//Push the task
			waiting_queue=remove_node(waiting_queue, temp);		//Remove the task from the waiting queue
			temp = waiting_queue;								//Set our iterator to the new waiting queue
		}
		else
		{
			temp = temp->next;									//Iterate the list...
		}
	}
}



//------------------Scheduler, returns the task to be executed ------------------
task * scheduler_n()
{
	if (ready_queue != NULL)			//If the ready queue isn't empty, we have tasks ready to be returned from the scheduler
	{
		if (sched_type == sched_RR) 		//Here is the round robin (RR) scheduler, in the RR case, we just return the first element of the ready queue
		{
			return ready_queue;
		}
		if (sched_type == sched_SJF) 		//Here is where you implement your EDF scheduling algorithm
		{
			quickSort(&ready_queue);
			return ready_queue;
		}
		if (sched_type == sched_MQ) 		//Here is where you implement your MQ scheduling algorithm,
		{
			task * current;

			for(current = ready_queue; current != NULL; current = current->next){
				if(current->quantum < 2){
					high_priority = push(high_priority, *current);
					
					printf("Inserting High\n");
				}
				else if(current->quantum > 1 && current->quantum < 4){
					medium_priority = push(medium_priority, *current);

					printf("Inserting Medium\n");
				}
				else{
					low_priority = push(low_priority, *current);
					
					printf("Inserting Low\n");
				}
			}
			if(high_priority != NULL){
				ready_queue = high_priority;
				printf("High\n");
			}
			else if(medium_priority != NULL){
				ready_queue = medium_priority;
				
				printf("Medium\n");
			}
			else{
				ready_queue = low_priority;
				printf("Low\n");
			}
			
			return ready_queue;
		}
	}
	else						//If the ready queue is empty, the operating system must have something to do, therefore we return an idle task
	{
		idle_task->quantum++;			//Make sure that the idle task dosnt run out of quantum
		printf("RETURNED IDLE TASK ");
		return idle_task;			//Return the idle task
	}
	return NULL;
}
//------------------ Dispatcher executes the task ------------------
void dispatch_n(task* exec)
{
	if (exec != exec_task) //If the pointer of the last executing task is not equal to the newly executing task a context switch has occurred
	{
		context_switches++; //Increase the context switches
		exec_task = exec; //Change the pointer of the currently executing task
	}

	exec->quantum--; //Decrease the time quantum of a task, i.e. run the task

	if (exec->quantum > 0)
	{
		if (exec == idle_task)
		{
			// Print
			printf("Idle task is executing - Total context switches: %d \n", context_switches);
		}
		else
		{
			// Print task info
			printf("Task %d is executing with %d quanta left - Total context switches: %d \n", exec->ID, exec->quantum, context_switches);
		}

		ready_queue = first_to_last(ready_queue); //Re-sort ready queue
	}
	else
	{
		printf("Task %d has executed and finished its quanta - Total context switches: %d \n", exec->ID, context_switches);	//Printout task info
		ready_queue->release_time = ready_queue->release_time+ready_queue->period;
		//waiting_queue = push(waiting_queue, *ready_queue); 			//Add the finished task to the waiting queue
		ready_queue = pop(ready_queue); 						//Pop the finished task from ready queue
	}
}

int main(int argc, char **argv)
{
	char * fp = "hej";			//File path to taskset
	readTaskset_n(fp); 			//Read taskset
	task * task_to_be_run; 		//Return taskset from scheduler
	idle_task = create(1337, 0, 0, 0, 0, 200000000, NULL);  //Create a dummy idle task
	while (1)
	{
		OS_wakeup_n();						//Wake up sleeping tasks
		task_to_be_run = scheduler_n();		//Fetch the task to be run
		dispatch_n(task_to_be_run);			//Dispatch the task to be run
		OS_cycles++;						//Increment OS clock
		usleep(1000000);					//Sleep so we dont get overflown with output
	}
}
