#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "testlara.h"

pthread_mutex_t 	roomLock; 
pthread_cond_t 		occupancy; 
unsigned int 		maximum, current, studentID, numOfThread, numOfStudents;
sem_t			question, profAvail, done;		

void
EnterOffice( int id ) {
	  pthread_mutex_lock( &roomLock );
	  while( current == maximum )
	    	pthread_cond_wait( &occupancy, &roomLock );
	  printf("Student %d shows up in the office.\n", id);
	  current++;
	  pthread_mutex_unlock( &roomLock );
}

void
LeaveOffice( int id ) {
  	pthread_mutex_lock( &roomLock );
  	current--;
  	printf("Student %d leaves the office.\n", id);
  	pthread_mutex_unlock( &roomLock );
	pthread_cond_signal( &occupancy );
}

void
QuestionStart( int id ) {
	studentID = id;
	printf("Student %d asks a question.\n", id);
}

void
AnswerStart() {
	printf("Professor starts to answer question for student %d.\n", studentID);
}

void
AnswerDone() {
	printf("Professor is done with answer for student %d.\n", studentID);
}

void
QuestionDone( int id ) {
	printf("Student %d is satisfied.\n", id);
}

void *
Professor() {
	while( numOfStudents ) {
		sem_wait( &question );
		AnswerStart();
		AnswerDone();
		sem_post( &done );		
	}
	pthread_exit(NULL);
}

void *
Student( void *arg) {
	int index, id = ( (int ) arg ); 
	EnterOffice(  id );
	for(index  = 0; index < ( id % 4 ) + 1; index++) {
		sem_wait( &profAvail );
		QuestionStart(  id );
		sem_post( &question );	
		sem_wait( &done );
		QuestionDone( id );
		sem_post( &profAvail );
	}
	LeaveOffice( id );
	numOfStudents--;
	pthread_exit( NULL );
}

void
_my_init() {
	if ( pthread_mutex_init( &roomLock, NULL ) ) {
		printf("Error: Unable to intialize Mutex\n");
		exit(0);
	}
	if ( pthread_cond_init( &occupancy, NULL ) ) {
		printf("Error: Unable to intialize Condition Vatiable\n");
		exit(0);
	}
	if ( sem_init( &question, 0, 0 ) | sem_init( &profAvail, 0, 1 ) | sem_init( &done, 0, 0 ) ) {
		printf("Error: Unable to intialize Semaphores\n");
		exit(0);
	}
	current = 0;
	numOfThread = numOfStudents;
}

void
_my_exit() {
	sem_destroy( &question );
	sem_destroy( &profAvail );
	sem_destroy( &done );
	pthread_mutex_destroy( &roomLock );
	pthread_cond_destroy( &occupancy );
  	exit(0);
}
int
main( int argc, char* argv[] ) {
	if ( argc != 3)
		printf("Usage: office [capacity of office] [number of students]");
	maximum = atoi( argv[1] );
	numOfStudents = atoi( argv[2] );
	_my_init();
	int index, nthr;
	pthread_t threads[numOfThread], prof;
	pthread_create( &prof , NULL , Professor , NULL);
	for (index = 0; index < numOfThread; index++) {
		if((nthr=pthread_create( &threads[index] , NULL , Student , (void *) index ) ) != 0) {
			printf("Error in thread creation: %d\n", nthr);
			exit(-1);
		}
	}
	for(index = 0; index < numOfThread; index++) {
		if ( (nthr = pthread_join(threads[index], NULL) ) != 0 ) {
			printf("Error in joining thread: %d\n", nthr);
			exit(-1);
		}
	}
	_my_exit();
}
