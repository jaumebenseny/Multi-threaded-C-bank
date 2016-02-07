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

#define MAXSTRING 650
#define MAXDESK 2
#define MAXCLIENTS 15
#define LOGFILE	"log.txt"

typedef struct {	
	pthread_mutex_t	qmutex; /* QUEUE MUTEX - controls atomic access to qLenght[] */
	pthread_mutex_t dmutex[MAXDESK]; /* DESK MUTEX controls atomic access to rows of messageQueue */
	int qLenght[MAXDESK]; /* Number of clients waiting for each desk. CONTROLED BY QMUTEX */
	char *messageQueue[MAXDESK][MAXCLIENTS]; /* Commands delivered by clients- CONTROLED BY DMUTEX[] */
	int exit[MAXDESK]; /* exits the program CONTROLED BY DMUTEX[] */
	int balance[2][MAXDESK]; /* Positon 0 control, position 1 returned balance from the desks to main thread. CONTROLED BY DMUTEX */
	pthread_cond_t	cond[MAXDESK]; /* thread synchronization between clients and desks */
	pthread_rwlock_t rwlock[MAXCLIENTS];
	int account[MAXCLIENTS];
	pthread_rwlock_t logBlock;
	pthread_barrier_t barrier;

} my_array_t;

extern my_array_t *thread_array;
