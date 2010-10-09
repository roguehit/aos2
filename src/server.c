#include <sys/types.h>

#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>

#include <sys/shm.h>

/*--------------------------------------+
| ISO/ANSI header files                 |
+--------------------------------------*/
#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <time.h>

#include <errno.h>

/*--------------------------------------+
| Constants                             |
+--------------------------------------*/
/* value to fill memory segment        */
#define MEM_CHK_CHAR         '$'    

/* shared memory key                   */
#define SHM_KEY      (key_t)1097    

/* size of memory segment (bytes)      */
#define SHM_SIZE     (size_t)256   

/* give everyone read/write permission   */
/* to shared memory                      */
#define SHM_PERM  (S_IRUSR|S_IWUSR \
  |S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)


void print_prime(char *prime)
{
        int i;
        for(i = 0; i < strlen(prime) && prime[i] == '0'; i++);
        for(; i < strlen(prime); i++)
                printf("%c", prime[i]);
        printf("\n\n\n");
}


/*----------------------------------------+
| Name:      main                         |
| Returns:   exit(EXIT_SUCCESS) or        |
|            exit(EXIT_FAILURE)           |
+-----------------------------------------*/
int main()
{
/* shared memory segment id           */
    int shMemSegID;             

/* shared memory flags                */
    int shmFlags;               

/* ptr to shared memory segment       */
    char * shMemSeg;            

/*---------------------------------------*/
/* Create shared memory segment          */
/* Give everyone read/write permissions. */
/*---------------------------------------*/
   shmFlags = IPC_CREAT | SHM_PERM;

 if ( (shMemSegID = 
  shmget(SHM_KEY, SHM_SIZE, shmFlags)) < 0 )
   {
       perror("SERVER: shmget");
       exit(EXIT_FAILURE);
   }

/*-------------------------------------------*/
/* Attach the segment to the process's data  */
/* space at an address                       */
/* selected by the system.                   */
/*-------------------------------------------*/
shmFlags = 0;
if ( (shMemSeg = 
      shmat(shMemSegID, NULL, shmFlags)) == 
  (void *) -1 )
   {
       perror("SERVER: shmat");
       exit(EXIT_FAILURE);
   }

/*-------------------------------------------*/
/* Fill the memory segment with MEM_CHK_CHAR */
/* for other processes to read               */
/*-------------------------------------------*/
   memset(shMemSeg, MEM_CHK_CHAR, SHM_SIZE);

/*-----------------------------------------------*/
/* Go to sleep until some other process changes  */
/* first character                               */
/* in the shared memory segment.                 */
/*-----------------------------------------------*/
   while (*shMemSeg == MEM_CHK_CHAR)
   {    
    //fprintf(stderr,"%c ",shMemSeg[0]);
    sleep(1);
   }
   //sleep(5);
   print_prime(shMemSeg);
/*------------------------------------------------*/
/* Call shmdt() to detach shared memory segment.  */
/*------------------------------------------------*/
   if ( shmdt(shMemSeg) < 0 )
   {
       perror("SERVER: shmdt");
       exit(EXIT_FAILURE);
   }


/*--------------------------------------------------*/
/* Call shmctl to remove shared memory segment.     */
/*--------------------------------------------------*/
   if (shmctl(shMemSegID, IPC_RMID, NULL) < 0)
   {
       perror("SERVER: shmctl");
       exit(EXIT_FAILURE);
   }
 
   exit(EXIT_SUCCESS);

}  /* end of main() */

