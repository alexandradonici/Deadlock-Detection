

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lockcheck.h"

#define NUM_THREADS	5
#define NUM_TEST	5

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
//exemplul de deadlock din curs C7.16
void *taskA(void *threadid)
{
long tid;
   tid = (long)threadid;
   printf("task A! It's me, thread #%ld!\n", tid);


 mutex_lock(&mutex1); 
           mutex_lock(&mutex2);
           mutex_unlock(&mutex2);
           mutex_unlock(&mutex1);

   pthread_exit(NULL);  

}

void *taskB(void *threadid)
{
long tid;
   tid = (long)threadid;
   printf("task B! It's me, thread #%ld!\n", tid);


   mutex_lock(&mutex2); 
   mutex_lock(&mutex1);
   mutex_unlock(&mutex1);
   mutex_unlock(&mutex2);

   pthread_exit(NULL);
}
int no_deadlock1();

int main()
{
   pthread_t threads[NUM_THREADS];
   int rc;
   long t;
   t = 0;
   rc = pthread_create(&threads[t], NULL, taskA, (void *)t);
   t++;
   rc = pthread_create(&threads[t], NULL, taskB, (void *)t);
   if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
   }
        pthread_join(threads[0],NULL);
	pthread_join(threads[1],NULL);
if(no_deadlock1()==1)
        printf("NO DEADLOCK\n");
else
printf("There is deadlock\n");


	pthread_exit(NULL);

	return 0;
}


