#include "global_header.h" 
#include "genlib.h" 

my_array_t *thread_array;

/* it returns the amount to be added to the balance of the desk */
long processCommand(char * command){
	
	char tmp[MAXSTRING] = ""; /* used for comparing */
	char tmp2[MAXSTRING] = "";
        char tmp3[MAXSTRING] = ""; /* used for comparing and for log */
	char string[MAXSTRING] = "";
	char *ch, *end,*save_ptr;
	int a1, a2, a3;

	strcpy(tmp,command);
	ch = strtok_r(tmp," ",&save_ptr);
	if(ch == NULL) return (1.5);

	sprintf(tmp3,"PROCESS COMMAND : %s -> ",command);

	if(*ch == 'd'){
		a1 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		a2 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		sprintf(tmp2,"DEPOSIT %d TO ACCOUNT %d\n",a2,a1);
		if(pthread_rwlock_wrlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);			
		}
		thread_array->account[a1] = thread_array->account[a1] + a2;
		if(pthread_rwlock_unlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: UNLOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);		
		}
		strcat(tmp3,tmp2);
		printf("\n*** SERVANT: %s)",tmp2);
		Log(tmp3);
		return(a2);
	}
	else if(*ch == 'l'){
		a1 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		if(pthread_rwlock_rdlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);		
		}
		sprintf(tmp2,"*** BALANCE ACCOUNT %d EQUALS %d \n",a1,thread_array->account[a1]);
		if(pthread_rwlock_unlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: UNLOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);
		}
		strcat(tmp3,tmp2);
		printf("\n*** SERVANT: %s)",tmp2);
		Log(tmp3);
		return(0);
	}
	else if(*ch == 'w'){
		a1 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		a2 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		sprintf(tmp2,"WITHDRAW %d FROM ACCOUNT %d\n",a2,a1);
		if(pthread_rwlock_wrlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);
		}
		thread_array->account[a1] = thread_array->account[a1] - a2;
		if(pthread_rwlock_unlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: UNLOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);
		}
		strcat(tmp3,tmp2);
		printf("\n*** SERVANT: %s)",tmp2);
		Log(tmp3);
		return(a2*-1);
	}
	else if(*ch == 't'){
		a1 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		a2 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		a3 = strtol(strtok_r(NULL," ",&save_ptr), &end, 10);
		sprintf(tmp2,"TRANSFER %d FROM ACCOUNT %d TO ACCOUNT %d\n",a3,a1,a2);
		if(pthread_rwlock_wrlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);
		}
		thread_array->account[a1] = thread_array->account[a1] - a3;
		if(pthread_rwlock_unlock(&thread_array->rwlock[a1]) != 0){
			strcpy(string,"ERROR: UNLOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);		
		}
		if(pthread_rwlock_wrlock(&thread_array->rwlock[a2]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);
		}
		thread_array->account[a2] = thread_array->account[a2] + a3;
		if(pthread_rwlock_unlock(&thread_array->rwlock[a2]) != 0){
			strcpy(string,"ERROR: LOCK RWERROR CAN'T PROCESS COMMAND \n");		
			Log(string);		
		}
		strcat(tmp3,tmp2);
		printf("\n*** SERVANT: %s)",tmp2);
		Log(tmp3);
		return(0);
	}
	return(1.5);
}
/* desk thread function */
void *thread_routine_desk(void * arg) 
{
	int last_command = 0;
	int desk = (int) arg;
	int res, tmp, exit = 0;
	int balance = 0;
	char string[MAXSTRING];

	while (exit == 0){	

		/* DESK MUTEX processing new messages */
	  	if(pthread_mutex_lock(&thread_array->dmutex[desk]) != 0){
			strcpy(string,"ERROR: SERVANT CAN'T LOCK MUTEX \n");		
			Log(string);			
		}
		/* if balance is requested and not reported yet, we report */	
		if(thread_array->balance[0][desk] == 1 && thread_array->balance[1][desk] == -1 ){
			thread_array->balance[1][desk] = balance;
			if(pthread_mutex_unlock(&thread_array->dmutex[desk]) != 0){
				strcpy(string,"ERROR: SERVANT CAN'T UNLOCK MUTEX \n");		
				Log(string);
			}
			/* while all desks haven't reported we sleep */
			res = pthread_barrier_wait(&thread_array->barrier);
  			if( res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD ) {
				strcpy(string,"ERROR: BARRIER RES ERROR\n");		
				Log(string);	
			}
			if(pthread_mutex_lock(&thread_array->dmutex[desk]) != 0){
			strcpy(string,"ERROR: SERVANT CAN'T LOCK MUTEX \n");		
			Log(string);			
			} 
		}
		/* while there are no new messages pending to be processed, we sleep */
		while (thread_array->messageQueue[desk][last_command] == NULL){
			if(thread_array->exit[desk] == 1) {/* wake up, program is ending */
				exit = 1;
				break;
			} 
			if(thread_array->balance[0][desk] == 1) {
				break;	/* wake up, balance is required */
			}
			res = pthread_cond_wait(&thread_array->cond[desk],&thread_array->dmutex[desk]);
			if(res == 0){
				strcpy(string,"ERROR: COND WAIT ERROR\n");		
				Log(string);	
			}
		}
		/* if command is not required to be processed */
		if(thread_array->exit[desk] == 1 || thread_array->balance[0][desk] == 1){
			if(pthread_mutex_unlock(&thread_array->dmutex[desk]) != 0){
				strcpy(string,"ERROR: SERVANT CAN'T LOCK MUTEX \n");		
				Log(string);
			}
		}
		/* if command requires to be processed */
		else{
			sprintf(string,"INFO: Thread %lu - SERVANT IN DESK %d - ", pthread_self(),desk);
			Log(string);
			tmp = processCommand(thread_array->messageQueue[desk][last_command]);
			if(tmp == 1.5){
				strcpy(string,"ERROR: SERVANT CAN'T PROCESS COMMAND \n");		
				Log(string);
			}
			else balance = balance + tmp;			
			thread_array->messageQueue[desk][last_command] = "1";
			last_command++;
	  		if(pthread_mutex_unlock(&thread_array->dmutex[desk]) != 0){
				strcpy(string,"ERROR: SERVANT CAN'T UNLOCK MUTEX \n");		
				Log(string);
			}
		}
	}
	pthread_exit(arg);
}

