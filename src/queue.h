#define MAXLENGTH 5
#define TRUE 1
#define FALSE 0

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <signal.h>

#define MAX_CLIENT 50
/*
This contains the PID of the client
If client[x]!=0, Server attaches to the clients shared memory 
Its SH_KEY = PID of Client 
*/
typedef struct __global_mem
{
pid_t server;
pid_t client[MAX_CLIENT]; 
}global_mem;

pid_t* client;

typedef struct __task
{
	int clientid;
	char p[256]; /*Exponent */ 
	char m[256]; /*Prime Number*/
	char response[256]; /*Response*/
}Task;

struct queue
{
	Task elements[MAXLENGTH];
	int head;
	int tail;
};
/*initialize q*/

void initialize_queue(struct queue *q)
{
	q->head = MAXLENGTH-1;
	q->tail = MAXLENGTH-1;
}

int is_empty(struct queue *q)
{
	return (q->tail == q->head) ? TRUE : FALSE;
}
void enqueue(struct queue *q, Task element)
{
	if (q->tail == MAXLENGTH-1)
		q->tail = 0;
	else
		(q->tail)++;
	
	if(q->tail == q->head){
		printf("Queue is full");
		exit(1);
	}
	q->elements[q->tail] = element;	
	printf("Inserted request from Thread %d\n", element.clientid);	
	
}

Task dequeue(struct queue *q)
{
	Task element;	
	if(is_empty(q)){
		printf("%s", "Queue is empty");
		exit(1);
	}
	if(q->head == MAXLENGTH-1)
		q->head = 0;
	else
		(q->head)++;
	element = q->elements[q->head];
	printf("\nRemoved an a request from thread %d\n", element.clientid);
	return element;
}
void print(struct queue *q)
{
	int i = 0;
	while(i<MAXLENGTH)
	{
		printf("%d",q->elements[i].clientid);
		i++;
	}
	printf("\nq->head = %d,q->tail = %d\n",q->head, q->tail);	
}

void create_task(Task *task, int clientid, char* p, char* m)
{
	task->clientid	=  clientid;
	strcpy(task->p,p);
	strcpy(task->m,m);
}

extern void block_signal(int signo)
{
        sigset_t set;

        /* Block the signal */
        sigemptyset(&set);
        sigaddset(&set, signo);
        sigprocmask(SIG_BLOCK, &set, NULL);

        return;
}

extern void unblock_signal(int signo)
{
        sigset_t set;

        /* Unblock the signal */
        sigemptyset(&set);
        sigaddset(&set, signo);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        return;
}


