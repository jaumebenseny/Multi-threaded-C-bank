#include "global_header.h" 
#include "genlib.h" 


my_array_t *thread_array;

/* returns shortest queue available */
int getShortestQueue(int * qLenght)
{
	int i, min, shortestQueue;
	min = qLenght[0];
	shortestQueue = 0;
	for(i = 0; i < MAXDESK; i++){
		if(qLenght[i] <	min) {
			min = qLenght[i];
			shortestQueue = i;	
		}	
	}
	qLenght[shortestQueue] = qLenght[shortestQueue] + 1;
	return shortestQueue;
}
void printAccounts(){
	int i;
	char string[MAXSTRING];
 	
	printf("*** ACCOUNTS");
	for(i = 0; i < MAXCLIENTS; i++){
		if(pthread_rwlock_rdlock(&thread_array->rwlock[i])!= 0){
			strcpy(string,"ERROR: RWERROR CAN'T PRINT ACCOUNTS \n");		
			Log(string);
		}
		printf(" / %d / ",thread_array->account[i]);
		if(pthread_rwlock_unlock(&thread_array->rwlock[i]) != 0){
			strcpy(string,"ERROR: RWERROR CAN'T PRINT ACCOUNTS \n");		
			Log(string);
		}
	}
	printf("***\n");
}
void requestBalance(){

	int i,balance = 0, res;	
	char string[MAXSTRING];
	
	/* we initalize the barrier */
	if(pthread_barrier_init(&thread_array->barrier,NULL,MAXDESK+1) != 0){
		strcpy(string,"ERROR: BARRIER ERROR \n");		
		Log(string);
	}

	/* we signal balance flag and wake up all servants from DESK MUTEX */
	for(i=0;i<MAXDESK;i++){
		if(pthread_mutex_lock(&thread_array->dmutex[i])!= 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T LOCK MUTEX \n");		
			Log(string);			
		}
		thread_array->balance[0][i] = 1;
		if(pthread_cond_broadcast(&thread_array->cond[i]) != 0 || pthread_mutex_unlock(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T UNLOCK MUTEX \n");		
			Log(string);			
		}
	}
	/* client joins the barrier */
	res = pthread_barrier_wait(&thread_array->barrier);
  	if( res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD ) {
		strcpy(string,"ERROR: BARRIER RES ERROR\n");		
		Log(string);	
	} 
	/* we get the balance of all queues and balance flag to 0 */
	for(i=0;i<MAXDESK;i++){
		if(pthread_mutex_lock(&thread_array->dmutex[i])!= 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T LOCK MUTEX \n");		
			Log(string);		
		}
			if(thread_array->balance[1][i] != -1){
				balance = balance + thread_array->balance[1][i];
				printf("INFO: BALANCE FROM TABLE %d IS: %d ---\n",i,thread_array->balance[1][i]);
			}
	  	if (pthread_cond_broadcast(&thread_array->cond[i]) != 0 || pthread_mutex_unlock(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T UNLOCK MUTEX \n");		
			Log(string);			
		}
	}
	/* we signal balance flag to 0 and wake up all servants from DESK MUTEX */
	for(i=0;i<MAXDESK;i++){
		if(pthread_mutex_lock(&thread_array->dmutex[i])!= 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T LOCK MUTEX \n");		
			Log(string);
		}
		thread_array->balance[0][i] = 0;
		thread_array->balance[1][i] = -1;
		if (pthread_cond_broadcast(&thread_array->cond[i]) != 0 || pthread_mutex_unlock(&thread_array->dmutex[i]) != 0){
			strcpy(string,"ERROR: MAIN THREAD CAN'T UNLOCK MUTEX \n");		
			Log(string);
		}
	}
	if(pthread_barrier_destroy(&thread_array->barrier) != 0){
		strcpy(string,"ERROR: DESTROIYING BARRIER \n");		
		Log(string);		
	}
}
/* it returns the amount to be added to the balance of the desk */
int checkCommand(char * command){
	
	char tmp[MAXSTRING] = ""; /* used for comparing */
	char *ch, *end,*save_ptr,*aux;
	int a1, a2, a3;

	strcpy(tmp,command);
	ch = strtok_r(tmp," ",&save_ptr);
	if (ch == NULL) return(0);

	if(*ch == 'd'){
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a1 = strtol(aux, &end, 10);
		if(a1 > MAXCLIENTS) return(0);
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a2 = strtol(aux, &end, 10);
		if ( (a1 || a1 == 0) && (a2 || a2 ==0) ) return(1);
	}
	else if(*ch == 'l'){
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a1 = strtol(aux, &end, 10);
		if(a1 > MAXCLIENTS) return(0);
		if ( a1 || a1 == 0) return(1);
	}
	else if(*ch == 'w'){
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a1 = strtol(aux, &end, 10);
		if(a1 > MAXCLIENTS) return(0);
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a2 = strtol(aux, &end, 10);
		if ( (a1 || a1 == 0) && (a2 || a2 ==0) ) return(1);
	}
	else if(*ch == 't'){
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a1 = strtol(aux, &end, 10);
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a2 = strtol(aux, &end, 10);
		aux = strtok_r(NULL," ",&save_ptr);
		if (aux == NULL) return(0);
		a3 = strtol(aux, &end, 10);
		if(a1 > MAXCLIENTS || a2 > MAXCLIENTS) return(0);
		if ( (a1 || a1 == 0) && (a2 || a2 ==0) && (a3 || a3 ==0) ) return(1);
	}
	return(0);
}
/* clients thread function */
void *thread_routine_clients(void * arg) 
{
	int desk, i = 0, escape = 0;
	int counter = (int) arg;
	char string[MAXSTRING],command[MAXSTRING];

	/* the client gets the number of shortest queue by GETING LOCK OF QMUTEX */
	if(pthread_mutex_lock(&thread_array->qmutex) != 0){
		strcpy(string,"ERROR: CLIENT CAN'T LOCK MUTEX \n");		
		Log(string);
	}	
	desk = getShortestQueue(thread_array->qLenght);
	printf("WELCOME CLIENT %d\n", counter);
	/* the users is asked to give command */
	while(1){
		printf("Please, introduce new command: ");
		if(fgets(command, sizeof(command), stdin) == NULL){
			strcpy(string,"ERROR: FGETS ERROR \n");		
			Log(string);
		}
		*(command + sizeof(command) -1 ) = '\0';
		if(checkCommand(command) == 0){
			if(*(command) == '0'){
				escape = 1; /* exit condition */
				break;
			}
			else if(*(command) == '1') {
				requestBalance(); /* balance condition */
			}
			else if(*(command) == '2') {
				printAccounts(); /* balance condition */
			}
			else printf("ERROR: Incorrect command\n");
		}
		else break;
	}
	/* we unlock the QUEUE MUTEX */
	if(pthread_mutex_unlock(&thread_array->qmutex) != 0){
		strcpy(string,"ERROR: CLIENT CAN'T UNLOCK MUTEX \n");		
		Log(string);
	}
	if(escape == 0){
		/* the client accesses the assigned queue */
	  	if(pthread_mutex_lock(&thread_array->dmutex[desk]) != 0){
			strcpy(string,"ERROR: CLIENT CAN'T LOCK MUTEX \n");		
			Log(string);	
		}
		/* client delivers its petition */
		while (thread_array->messageQueue[desk][i] != NULL){
			i++;
		}

		thread_array->messageQueue[desk][i] = command; 
		sprintf(string,"INFO: Thread %lu - CLIENT IN DESK %d, COMMAND %s, POSITION %d \n",pthread_self(),desk, command,i);
		Log(string);
		/* we signal new message to the queue to everybody, but only the desk checks */    	
		if(pthread_cond_broadcast(&thread_array->cond[desk]) != 0 || pthread_mutex_unlock(&thread_array->dmutex[desk]) != 0){
			strcpy(string,"ERROR: CLIENT CAN'T UNLOCK MUTEX \n");		
			Log(string);
		}
	}
	pthread_exit(arg);
}
