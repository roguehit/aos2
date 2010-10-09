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
/*-------------------------------------+
| Constants                            |
+-------------------------------------*/
/* memory segment character value     */
#define MEM_CHK_CHAR        '$'     
#define MAX_THREAD 50
/* shared memory key                  */
#define SHM_KEY      (key_t)1097    

#define SHM_SIZE     (size_t)256    

/* size of memory segment (bytes)     */
/* give everyone read/write           */
/* permission to shared memory        */
#define SHM_PERM  (S_IRUSR|S_IWUSR\
  |S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
int num_threads;
int frequency;
/*------------------------------------+
| Name:      main                     |
| Returns:   exit(EXIT_SUCCESS) or    |
|            exit(EXIT_FAILURE)       |
+------------------------------------*/
void status() {}
void print_prime(char *prime)
{
        int i;
        for(i = 0; i < strlen(prime) && prime[i] == '0'; i++);
        for(; i < strlen(prime); i++)
                printf("%c", prime[i]);
        printf("\n");
}

void* generate_request(void*tid)
{

    char *prime;
    BIGNUM *num_tmp;
    long int num_bits = 512;
/* loop counter                       */
    int i;                      

/* shared memory segment id           */
    int shMemSegID;             

/* shared memory flags                */
    int shmFlags;               

/* ptr to shared memory segment       */
    char * shMemSeg;            

/* generic char pointer               */
    char * cptr;                

/*-------------------------------------*/
/* Get the shared memory segment for   */
/* SHM_KEY, which was set by           */
/* the shared memory server.           */
/*-------------------------------------*/
   shmFlags = SHM_PERM;

if ( (shMemSegID = 
shmget(SHM_KEY, SHM_SIZE, shmFlags)) < 0 )
   {
   perror("CLIENT: shmget");
  printf("ret val =%d\n",shMemSegID);
   exit(EXIT_FAILURE);
   }

/*-----------------------------------------*/
/* Attach the segment to the process's     */
/* data space at an address                */
/* selected by the system.                 */
/*-----------------------------------------*/
shmFlags = 0;
if ( (shMemSeg = 
      shmat(shMemSegID, NULL, shmFlags)) == 
      (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }

/*-------------------------------------------*/
/* Read the memory segment and verify that   */
/* it contains the values                    */
/* MEM_CHK_CHAR and print them to the screen */
/*-------------------------------------------*/
#if 0
for (i=0, cptr = shMemSeg; i < SHM_SIZE; 
i++, cptr++)
   {
      if ( *cptr != MEM_CHK_CHAR )
      {
         fprintf(stderr, "CLIENT: emory Segment corrupted!\n");
         exit(EXIT_FAILURE);
      }

      putchar( *cptr );
/* print 40 columns across            */

      if ( ((i+1) % 40) == 0 )        
     {
         putchar('\n');
      }
   }
   putchar('\n');

  fprintf(stderr,"Verified\n");
#endif
/*--------------------------------------------*/
/* Clear shared memory segment.               */
/*--------------------------------------------*/
   num_tmp = BN_new();
  
   BN_generate_prime(num_tmp,num_bits,1,NULL,NULL,status,NULL);
   prime = (char *)malloc(BN_num_bytes(num_tmp));
   prime = BN_bn2dec(num_tmp);
   print_prime(prime);
   free(prime);
   BN_free(num_tmp);

   strcpy(shMemSeg,prime);


/*------------------------------------------*/
/* Call shmdt() to detach shared            */
/* memory segment.                          */
/*------------------------------------------*/
   if ( shmdt(shMemSeg) < 0 )
   {
       perror("SERVER: shmdt");
       exit(EXIT_FAILURE);
   }

   pthread_exit(0);

}  /* end of main() */


int main (int argc, char ** argv)
{

if(argc < 2){
fprintf(stderr,"Usage: ./client <num_of_threads> <reqs/thread/sec>\n");
exit(0);
}
 num_threads = atoi(argv[1]);
 frequency = atoi(argv[2]);
int i=0;

pthread_t thread[MAX_THREAD];

for(i=0;i<num_threads;i++){
pthread_create(&thread[i],NULL,generate_request,(void*)i);
//pthread_detach(thread[i]);
}
pthread_exit(0);
}
