/* Includes */
#include <stdio.h>      /* Input/Output */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <stdlib.h>     /* General Utilities */
#include <signal.h>	/* Signals */
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>

#include "global_header.h"
#include "clientslib.h"
#include "servantslib.h"
#include "genlib.h" 
 
my_array_t *thread_array; /* declared as extern in global_header.h */

int main(int argc, char *argv[])
{
	pthread_t clients_id[MAXCLIENTS];
	pthread_t desk_id[MAXDESK];
	char string[MAXSTRING];
	int i,accountSum = 0;
	void *thread_result;
	FILE *logfile;

	/* we initialize struct */
	if((thread_array = (my_array_t *)malloc(sizeof(my_array_t)))== NULL){
		strcpy(string,"ERROR: MAIN THREAD MALLOC FAILED\n");		
		printf("%s",string);	
	}
	memset(thread_array,0,sizeof(my_array_t));
	/* we initialize struct variables */
	for(i=0; i < MAXDESK; i++) {
		thread_array->qLenght[i] = 0 ; 	/* we initialize the queues */
		thread_array->exit[i] = 0 ; 	/* we initialize the exit */
		thread_array->balance[0][i] = 0; /* we initialize balances */
		thread_array->balance[1][i] = -1;
	
	}
	/* we create log file and we close it*/
	logfile = fopen(LOGFILE, "w");
	if (logfile == NULL){
	      	printf("ERROR: LOG FILE CAN'T BE CREATED\n");
	}
	else{
		strcpy(string,"START OF LOG FILE\n");
		fputs(string, logfile);
		if(fclose(logfile)!=0) printf("ERROR: LOG FILE CAN'T BE CREATED\n");
	}
	/* we initialize log condition */
	if(pthread_rwlock_init(&thread_array->logBlock,NULL) != 0){
		strcpy(string,"ERROR: MAIN THREAD RWLOCK INIT FAILED\n");		
		printf("%s",string);
	}
	/* we initialize accounts and locks */
	for(i=0; i < MAXCLIENTS; i++) {
		thread_array->account[i] = 0 ; 	/* we initialize the accounts */
		if(pthread_rwlock_init(&thread_array->rwlock[i],NULL) != 0){
			strcpy(string,"ERROR: MAIN THREAD RWLOCK INIT FAILED\n");		
			Log(string);
		}
	}
	/* we initialize mutexes and conditions */
	if(pthread_mutex_init(&thread_array->qmutex,NULL) != 0){
		strcpy(string,"ERROR: MAIN THREAD MUTEX INIT FAILED\n");		
		Log(string);
	}
	for(i=0; i < MAXDESK; i++) {
		if(pthread_mutex_init(&thread_array->dmutex[i],NULL) != 0){
			strcpy(string,"ERROR: MAIN THREAD MUTEX CREATION FAILED\n");		
			Log(string);
		}
		if(pthread_cond_init(&thread_array->cond[i],NULL)!= 0){ /* we initialize the conditions */
			strcpy(string,"ERROR: MAIN THREAD CONDITION INIT FAILED\n");		
			Log(string);		
		}
	}
	/* we create all servants */
	for(i=0; i < MAXDESK;i++) {
		if (pthread_create(&(desk_id[i]),NULL,thread_routine_desk,(void *)i) != 0){
			strcpy(string,"ERROR: THREAD CREATION FOR DESKS FAILED\n");		
			Log(string);
		}
	}
	/* *** BANK OPENS *** */
	printf("--------\nWELCOME TO UNIX BANK - CONFIGURATION OF THE BANK : CLIENTS: %d, DESKS: %d\n i) Type 0 to exit the client session \n ii) Type 1 to require balance from all desks\n iii) Type 2 to display accounts \n-------- \n",MAXCLIENTS,MAXDESK);
	printf("Examples of possible commands \n l 1: give balance of account 1\n w 1 123 : withdraw 123 euros from account 1\n t 1 2 123: transfer 123 euros from account 1 to account 2\n d 1 234: deposit 234 euros to account 1\n IMPORTANT: EACH USER CAN ONLY EXECUTE ONE COMMAND APART OF BALANCE, REQUEST\n--------\n");
	/* we create all clients */
	for(i=0; i < MAXCLIENTS;i++) {
		if (pthread_create(&(clients_id[i]),NULL,thread_routine_clients,(void *)i) != 0){
			strcpy(string,"ERROR: THREAD CREATION FOR CLIENTS FAILED\n");		
			Log(string);
		}
	}	
	/* we join all clients */
	for(i=0; i < MAXCLIENTS;i++) {
	    	if(pthread_join(clients_id[i],&thread_result) != 0){
			strcpy(string,"ERROR: THREAD JOIN FOR CLIENTS FAILED\n");		
			Log(string);		
		}
	}
	/* *** BANK CLOSES *** */
	/* we wake-up all servants */
	printf("INFO: BANK CLOSES \n");
	for(i=0;i<MAXDESK;i++){
		if(pthread_mutex_lock(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T LOCK MUTEX \n");		
			Log(string);
		}
		thread_array->exit[i] = 1;		
		if (pthread_cond_broadcast(&thread_array->cond[i]) != 0 || pthread_mutex_unlock(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T UNLOCK MUTEX \n");		
			Log(string);
		}
	}
	/* we join all desks */
	for(i=0; i < MAXDESK;i++) {
	    	if(pthread_join(desk_id[i],&thread_result) != 0){
			strcpy(string,"ERROR: THREAD JOIN FOR DESKS FAILED\n");		
			Log(string);		
		}
	}
	/* we destroy locks */
	for(i=0; i < MAXCLIENTS; i++) {
		accountSum = accountSum + thread_array->account[i];
		if(pthread_rwlock_destroy(&thread_array->rwlock[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD RWLOCK DESTROY FAILED\n");		
			Log(string);		
		}
	}
	printf("INFO: TOTAL ACCOUNT SUM : %d\n", accountSum);
	/* we destroy mutexes and conditions*/
	if(pthread_mutex_destroy(&thread_array->qmutex) != 0){
		strcpy(string,"ERROR: MAIN THREAD MUTEX DESTRUCTION FAILED\n");		
		Log(string);		
	}
	for(i=0; i < MAXDESK; i++) {
		if(pthread_mutex_destroy(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MUTEX DESTRUCTION FAILED\n");		
			Log(string);
		}
		if(pthread_cond_destroy(&thread_array->cond[i])!= 0){
			strcpy(string,"ERROR: MAIN THREAD COND DESTRUCTION FAILED\n");		
			Log(string);
		}
	}
	if(pthread_rwlock_destroy(&thread_array->logBlock) != 0){
		strcpy(string,"ERROR: MAIN THREAD COND DESTRUCTION FAILED\n");		
		printf("%s",string);
	}
	free(thread_array);
	printf("INFO: MAIN THREAD TERMINATING\n");
	return(0);
}
