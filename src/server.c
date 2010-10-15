/* POSIX/UNIX header files               |
+--------------------------------------*/
#include <sys/types.h>

#include <unistd.h>

#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include "queue.h"
/*--------------------------------------+
| ISO/ANSI header files                 |
+--------------------------------------*/
#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <time.h>

#include <errno.h>
#include <math.h>

#include <openssl/bn.h>

#include <errno.h>
#include <pthread.h>
#include <assert.h>
/*--------------------------------------+
| Constants                             |
+--------------------------------------*/

/* size of memory segment (bytes)      */
#define SHM_SIZE     (size_t)100*1024   

/* give everyone read/write permission   */
/* to shared memory                      */
#define SHM_PERM  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/*----------------------------------------+
| Name:      main                         |
| Returns:   exit(EXIT_SUCCESS) or        |
|            exit(EXIT_FAILURE)           |
+-----------------------------------------*/
global_mem *message_box;
int globalSegID; 

void SIGINT_handler (int);

void print_bignumber(char *number)
{
        int i,len;
	len=strlen(number);
	printf("Length is %d\n",len);
        for(i = 0; i < len && number[i] == '0'; i++);
        for(; i < strlen(number); i++)
                printf("%c", number[i]);
        printf("\n\n\n");
}

void compute_mod(Task *task);

void message_box_init ()
{
pid_t pid;
int i;
key_t MyKey;
/*Get the current process ID*/
pid = getpid();

/*Push the server PID into the message box for other clients to read */
MyKey   = ftok(".", 's');    
globalSegID   = shmget(MyKey, sizeof(pid_t), IPC_CREAT | 0666);
message_box  = (global_mem*) shmat(globalSegID, NULL, 0);

for(i=0;i<50;i++)
message_box->client[i]=0;

message_box->server = pid;

}

void* server_thread (void* t)
{
/* shared memory segment id           */
int MyKey;
int reqMemSegID;             
int resMemSegID;
/* shared memory flags                */
int shmFlags;               
    
/*pointer to the queue			*/
struct queue *request_ring;
struct queue *response_ring;


/*Keys to request & response structures*/
int REQ_KEY,RES_KEY;     

/*Get the Thread ID */
int tid =  (int)(int*)t;
MyKey   =  (int)message_box->client[tid];  
printf("Servicing Client with Process ID %d, Thread ID %d\n",(int)MyKey,tid);

/*Get the keys for both rings*/
REQ_KEY = ftok(".", (int) MyKey * 0xAA );    
RES_KEY = ftok(".", (int) MyKey * 0x33 );

/*Should not happen*/
assert(REQ_KEY != RES_KEY);

#if 1
/*---------------------------------------*/
/* Create shared memory segment          */
/* Give everyone read/write permissions. */
/*---------------------------------------*/

shmFlags = IPC_CREAT | SHM_PERM;
/*Request ring creation*/
 if ( (reqMemSegID = 
  shmget(REQ_KEY, SHM_SIZE, shmFlags)) < 0 )
   {
       perror("SERVER: shmget");
       exit(EXIT_FAILURE);
   }
printf("Created shared location for request ring buffer\n");
/*Response ring creation*/
if ( (resMemSegID = 
  shmget(RES_KEY, SHM_SIZE, shmFlags)) < 0 )
   {
       perror("SERVER: shmget");
       exit(EXIT_FAILURE);
   }
printf("Created shared location for response ring buffer\n");

/*-------------------------------------------*/
/* Attach the segment to the process's data  */
/* space at an address                       */
/* selected by the system.                   */
/*-------------------------------------------*/
/*Changed by Pradeep*/
shmFlags = 0;
/*Get Request Ring*/
if ( (request_ring = 
      shmat(reqMemSegID, NULL, shmFlags)) == 
  (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }
printf("Request Ring buffer attached to process\n");
/*Get Response Ring*/
if ( (response_ring = 
      shmat(resMemSegID, NULL, shmFlags)) == 
  (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }
printf("Response Ring buffer attached to process\n");


while(1){
	if(is_empty(request_ring))
	{
		//pthread_cond_wait(&cond_thread, &mutex_thread);
		sleep(1);
	}
	else if(!is_empty(request_ring))
	{	
		printf("Got a request, Dequeuing it \n");	
		Task temp = dequeue(request_ring);
		compute_mod(&temp);
		printf("Computed Mod\n");
		
	        enqueue(response_ring,temp);
	}
}
/*------------------------------------------------*/
/* Call shmdt() to detach shared memory segment.  */
/*------------------------------------------------*/
   if ( shmdt(request_ring) < 0 )
   {
       perror("SERVER: shmdt");
       exit(EXIT_FAILURE);
   }

  if ( shmdt(response_ring) < 0 )
   {
       perror("SERVER: shmdt");
       exit(EXIT_FAILURE);
   }

    

/*--------------------------------------------------*/
/* Call shmctl to remove shared memory segment.     */
/*--------------------------------------------------*/
   if (shmctl(reqMemSegID, IPC_RMID, NULL) < 0)
   {
       perror("SERVER: shmctl");
       exit(EXIT_FAILURE);
   }

  if (shmctl(resMemSegID, IPC_RMID, NULL) < 0)
   {
       perror("SERVER: shmctl");
       exit(EXIT_FAILURE);
   }
 

   exit(EXIT_SUCCESS);
#endif
}  /* end of main() */
/*--------------------------------------------------*/
/* Compute Modulus				   */
/*--------------------------------------------------*/
void compute_mod(Task * task)
{
	BN_CTX *context;
       	BIGNUM *r,*a,*p,*m;
        if(task->p==NULL){
	fprintf(stderr,"Wrong Exponent\n");
	exit(0);
        }
	/*Just printing out the client send numbers*/
 	printf("Printing the Exponent\n");	
        print_bignumber(task->p);
 	printf("Printing the Prime\n");	
	print_bignumber(task->m);
		
 	context = BN_CTX_new();
        r = BN_new();
	a = BN_new();
	p = BN_new();
	m = BN_new();

	BN_dec2bn(&a,"2");
	BN_dec2bn(&p,task->p);
	BN_dec2bn(&m,task->m);
	BN_mod_exp(r,a,p,m,context);

 	printf("Response ----\n");
	strcpy(task->response,BN_bn2dec(r));
	print_bignumber(task->response);
	
	return;
}

void  SIGINT_handler(int sig)
{
    
     block_signal(sig);
     int i;
     pthread_t thread;
//     signal(sig, SIG_IGN);
     printf("From SIGINT: just got a %d (SIGUSR1) signal\n", sig);
     
     for(i=0;message_box->client[i]!=0;i++);
     printf("Creating Thread for - Client with PID %d, Index: %d\n",(int)message_box->client[i-1],i-1);     
//   signal(sig, SIGINT_handler);

     pthread_create(&thread,NULL,server_thread,(void*) (i-1));
     pthread_detach(thread);

     unblock_signal(sig);
}

int main (int argc, char **argv)
{


    message_box_init();

/*Install Signal Handler to service I-am-alive messages from other clients*/
    if (signal(SIGUSR1, SIGINT_handler) == SIG_ERR) {
      printf("SIGINT install error\n");
      exit(1);
     }

    while(1){
	/*Sleep Infinitely & wait for Signals from Clients*/
	printf("Waiting ..\n");
	sleep(1);	
    }


/*Clean up for Global Message box*/
   if ( shmdt(message_box) < 0 )
   {
	perror("Could not Detach message_box");
	exit(EXIT_FAILURE);
   }

  if(shmctl(globalSegID, IPC_RMID, NULL) < 0)
   {
 	perror("Could not detach Global Segment ID");
	exit(EXIT_FAILURE);
   } 


return 0;
}
