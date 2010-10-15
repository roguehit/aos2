	/*-------------------------------------+
	| POSIX/UNIX header files              |
	+-------------------------------------*/
	#include <fcntl.h>

	#include <sys/types.h>

	#include <sys/ipc.h>

	#include <sys/shm.h>
	#include <sys/stat.h>
	/*-------------------------------------+
	| ISO/ANSI header files                |
	+-------------------------------------*/
	#include <stdlib.h>

	#include <stdio.h>

	#include <string.h>

	#include <errno.h>
	#include <openssl/bn.h>
	#include <errno.h>
	#include "queue.h"

	#include <pthread.h>
	#include <assert.h>
	/*-------------------------------------+
	| Constants                            |
	+-------------------------------------*/
	/* memory segment character value     */
	#define MEM_CHK_CHAR        '$'     
	#define MAX_THREAD 50

	#define SHM_SIZE      (size_t)600    

	/* size of memory segment (bytes)     */
	/* give everyone read/write           */
	/* permission to shared memory        */
	#define SHM_PERM  (S_IRUSR|S_IWUSR\
	  |S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

	int num_threads;
	int frequency;
	int number_of_seconds;

	/* ptr to queue */
	struct queue *request_ring;
	struct queue *response_ring;

	int reqMemSegID;
	int resMemSegID;

	void status() {}
	void print_prime(char *prime)
	{
		int i;
		for(i = 0; i < strlen(prime) && prime[i] == '0'; i++);
		for(; i < strlen(prime); i++)
			printf("%c", prime[i]);
		printf("\n");
	}

	void* generate_request(void* t)
	{    
	    srand(time(0));
	    char *prime;
	    BIGNUM *num_tmp;
	    char *p; //power or exponent
	    char *m; //modulus or prime number

	    long int num_bits = 512;
	/* Temporary Pointers  */
	    char *temp1,*temp2;
	
	    Task task;
	    int	tid=(int)(int*)t;
	    printf("Thread %d created\n",tid);
	/*-------------------------------------*/
	/* Get the shared memory segment for   */
	/* SHM_KEY, which was set by           */
	/* the shared memory server.           */
	/*-------------------------------------*/
	
	   /* a^r/p , Here a = 2, r & p = Big Prime Numbers */
	   num_tmp = BN_new();
	   printf("Generating Exponent\n");	
	   BN_generate_prime(num_tmp, num_bits+rand()%100+1 ,1,NULL,NULL,status,NULL);
	   temp1 =  (char *)malloc(BN_num_bytes(num_tmp));
	   temp1 = BN_bn2dec(num_tmp);
	   if(strcpy(task.p,temp1)==NULL){
	   fprintf(stderr,"Copy of exponent Failed\n");
	   exit(0);
	   }
	
	   print_prime(task.p);    
	   printf("Generating Prime\n");
	   BN_generate_prime(num_tmp,num_bits,1,NULL,NULL,status,NULL);
	   temp2 = (char *)malloc(BN_num_bytes(num_tmp));
	   temp2 = BN_bn2dec(num_tmp);
	   printf("Prime Created\n");
	   
	   if( strcpy(task.m,temp2)==NULL){
	   fprintf(stderr,"Copy of exponent Failed\n");
	   exit(0);
	   }
	

	   print_prime(task.m);   


	   task.clientid = tid;
	   //create_task(&task,tid,p,m); 
	   enqueue(request_ring,task); 
	      //free(prime);
	  // printf("Freeing Num_temp\n");
	   BN_free(num_tmp);
	  // printf("Freed Num_temp\n");
	   
	   while(1){}

	/*------------------------------------------*/
	/* Call shmdt() to detach shared            */
	/* memory segment.                          */
	/*------------------------------------------*/
	 

	   if ( shmdt(response_ring) < 0 )
	   {
	       perror("SERVER: shmdt");
	       exit(EXIT_FAILURE);
	   }

	   if ( shmdt(request_ring) < 0 )
	   {
	       perror("SERVER: shmdt");
	       exit(EXIT_FAILURE);
	   }


	   pthread_exit(0);

	} 

void init_ring()
{
	/*Keys to the Shared Ring Structures*/	
	int RES_KEY, REQ_KEY , MyKey;

	/* shared memory flags                */
	int shmFlags;
	
        printf("Size of task is %d\n",sizeof(Task));
	shmFlags = SHM_PERM;
	MyKey = (int)getpid();
	/*Get the keys for both rings*/
	REQ_KEY = ftok(".", (int) MyKey * 0xAA );    
	RES_KEY = ftok(".", (int) MyKey * 0x33 );

	/*Should not happen*/
	assert(REQ_KEY != RES_KEY);


	printf("Shared Mem for request\n");
	if ( (reqMemSegID =
	shmget(REQ_KEY, SHM_SIZE, shmFlags)) < 0 )
	   {
	   perror("CLIENT: shmget");
	   exit(EXIT_FAILURE);
	   }
	//Get Response Ring Buffer
	printf("	Shared Mem for response\n");
	if ( (resMemSegID =
	shmget(RES_KEY, SHM_SIZE, shmFlags)) < 0 )
	   {
	   perror("CLIENT: shmget");
	   exit(EXIT_FAILURE);
	   }

	/*-----------------------------------------*/
	/* Attach the segment to the process's     */
	/* data space at an address                */
	/* selected by the system.                 */
	/*-----------------------------------------*/
	printf("Request Ring Attached\n");
	
	if ( (request_ring = 
	      shmat(reqMemSegID, NULL, 0)) == 
	      (void *) -1 )
	   {
	       perror("SERVER: shmat");
	       exit(EXIT_FAILURE);
	   }
	printf("	Response Ring Attached\n");
	if ( (response_ring = 
	      shmat(resMemSegID, NULL, 0)) == 
	      (void *) -1 )
	   {
	       perror("SERVER: shmat");
	       exit(EXIT_FAILURE);
	   }

}

	int main (int argc, char ** argv)
	{
	int i=0;
	pid_t   server_pid, client_pid;
        key_t MyKey;
        int   ShmID,globalSegID;
        pid_t *ShmPTR;
	global_mem *message_box;	
			
	if(argc != 4){
	fprintf(stderr,"Usage: ./client <num_of_threads> <reqs/thread/sec> <number of seconds>\n");
	exit(0);
	}

	num_threads = atoi(argv[1]);
	frequency = atoi(argv[2]);
	number_of_seconds = atoi(argv[3]);	

	pthread_t thread[MAX_THREAD];

	MyKey          = ftok(".", 's');        
        ShmID          = shmget(MyKey, sizeof(pid_t), 0666);
        message_box    = (pid_t *) shmat(ShmID, NULL, 0);
        server_pid     = message_box->server;                
        //shmctl(ShmID, IPC_RMID, NULL);
	printf("Server's PID is %d\n",server_pid);
	client_pid = getpid();
  	printf("Client PID %d\n",client_pid);

	for(i=0;message_box->client[i]!=0;i++);
	printf("Index is %d\n",i);	
	message_box->client[i] = client_pid;
	
	
	/*Send a Signal to Server*/
	kill(server_pid, SIGUSR1);
	/*Sleep for sometime to make sure server thread is up & running*/	
	sleep(3);
	/*Initial*/
	init_ring();
#if 1
	for(i=0;i<num_threads;i++){
	pthread_create(&thread[i],NULL,generate_request,(void*)i);
	}
	pthread_exit(0);
#endif
	shmdt(message_box);

	return 0;
	}
