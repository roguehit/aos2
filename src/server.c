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
/*--------------------------------------+
| Constants                             |
+--------------------------------------*/
/* value to fill memory segment        */
#define MEM_CHK_CHAR         '*'    

/* shared memory key                   */
#define SHM_KEY      (key_t)1097    
#define SHM_KEY1     (key_t)1098  
#define SHM_KEY2     (key_t)1099
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
int main()
{
printf("Note to self do a Ctrl + C to stop erver.change this later\n");
/* shared memory segment id           */
    int reqMemSegID,i;             
    int resMemSegID;
    int globalSegID,ShmID;
/* shared memory flags                */
    int shmFlags;               
    
    pid_t pid;

/*pointer to the queue			*/
struct queue *request_ring;
struct queue *response_ring;


pid_t *pidPtr,*ShmPTR;
/* Declare Task 	*/
Task task;

key_t MyKey;
/*Get the current process ID*/
pid = getpid();

#if 0 
/* Get the Key, This Key will be same for Server & Clients */
if ( MyKey   = ftok(".", 's') == -1  )
{
fprintf(stderr,"Could not get the key value from ftok %s\n",strerror(errno));
exit(0);
}

shmFlags = IPC_CREAT | SHM_PERM;

//printf("Size of Shared mem %d\n",sizeof(global_mem));
/*Get the ID*/

if(globalSegID = shmget(MyKey,sizeof(pid_t), IPC_CREAT | 0666 ) < 0)
{
fprintf(stderr,"Could not get the Seg ID : %s\n",strerror(errno));
exit(0);
}

/*Attach the message box*/
pidPtr  = (pid_t*) shmat(globalSegID, NULL, 0);
#if 0
if (pidPtr  =  shmat(globalSegID, NULL, 0) == (void*) -1 )
{
fprintf(stderr,"Could not attach : %s\n",strerror(errno));
exit(0);
}
#endif

#endif
/*Push the server PID into the message box for other clients to read */
MyKey   = ftok(".", 's');    
ShmID   = shmget(MyKey, sizeof(pid_t), IPC_CREAT | 0666);
message_box  = (pid_t *) shmat(ShmID, NULL, 0);

for(i=0;i<50;i++)
message_box->client[i]=0;

message_box->server = pid;

//*pidPtr = pid;    

/*Install Signal Handler to service alive messages from other clients*/
if (signal(SIGUSR1, SIGINT_handler) == SIG_ERR) {
      printf("SIGINT install error\n");
      exit(1);
     }
     
/*---------------------------------------*/
/* Create shared memory segment          */
/* Give everyone read/write permissions. */
/*---------------------------------------*/
shmFlags = IPC_CREAT | SHM_PERM;
//Request ring creation
 if ( (reqMemSegID = 
  shmget(SHM_KEY, SHM_SIZE, shmFlags)) < 0 )
   {
       perror("SERVER: shmget");
       exit(EXIT_FAILURE);
   }
printf("Created shared location for request ring buffer\n");
//Response ring creation
if ( (resMemSegID = 
  shmget(SHM_KEY1, SHM_SIZE, shmFlags)) < 0 )
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
//Changed by Pradeep
shmFlags = 0;
//Get Request Ring
if ( (request_ring = 
      shmat(reqMemSegID, NULL, shmFlags)) == 
  (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }
printf("Request Ring buffer attached to process\n");
//Get Response Ring
if ( (response_ring = 
      shmat(resMemSegID, NULL, shmFlags)) == 
  (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }
printf("Response Ring buffer attached to process\n");

/*-------------------------------------------*/
/* Fill the memory segment with MEM_CHK_CHAR */
/* for other processes to read               */
/*-------------------------------------------*/
   //memset(shMemSeg, MEM_CHK_CHAR, SHM_SIZE);
	

/*-----------------------------------------------*/
/* Go to sleep until some other process changes  */
/* first character                               */
/* in the shared memory segment.                 */
/*-----------------------------------------------*/
 /*  while (*shMemSeg == MEM_CHK_CHAR)
   {
      sleep(1);
   }*/
//Go on forever
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
		//Get response ring
		//c+=1;
		/*Do not Enqueue Just yet*/
	        enqueue(response_ring,temp);
		//i++;
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

   if ( shmdt(message_box) < 0 )
   {
	perror("Could not Detach ShmPTR");
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

   if(shmctl(globalSegID, IPC_RMID, NULL) < 0)
   {
 	perror("Could not detach Global Segment ID");
	exit(EXIT_FAILURE);
   } 

   exit(EXIT_SUCCESS);

}  /* end of main() */
/*--------------------------------------------------*/
/* Compute Modulus				   */
/*--------------------------------------------------*/
void compute_mod(Task * task)
{
	BN_CTX *context;
       	BIGNUM *r,*a,*p,*m;
        if(p==NULL){
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
//     signal(sig, SIG_IGN);
     printf("From SIGINT: just got a %d (SIGUSR1) signal\n", sig);
     
     for(i=0;message_box->client[i]!=0;i++);
     printf("Client PID %d, Index: %d\n",(int)message_box->client[i-1],i-1);     
//     signal(sig, SIGINT_handler);
     unblock_signal(sig);
}


