#include "global_header.h" 

my_array_t *thread_array;

void Log(char * log){
	FILE *logfile;
	char string[MAXSTRING];

	if(pthread_rwlock_wrlock(&thread_array->logBlock) != 0){
		strcpy(string,"ERROR: LOCK RWERROR CAN'T RECORD LOG\n");		
		printf("%s",string);			
	}
	logfile = fopen(LOGFILE, "a");
	if (logfile == NULL){
	      	printf("ERROR: LOG FILE CAN'T BE CREATED\n");
	}
	fputs(log, logfile);
	if(fclose(logfile)!=0) printf("ERROR: LOG FILE CAN'T BE CREATED\n");

	if(pthread_rwlock_unlock(&thread_array->logBlock) != 0){
		strcpy(string,"ERROR: UNLOCK RWERROR CAN'T RECORD LOG \n");		
		printf("%s",string);		
	}
}
/* no more clients at ALL in any queue */
int noMoreClients(int * qLenght){
	int i;
	for(i = 0; i < MAXDESK; i++){
		if(qLenght[i] > 0) {
			return(0);	
		}	
	}
	return(1);
}
