#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;

void *contar(void *threadid) {
  int thread_id = *((int *)threadid);
  int i;

  for (i = 0; i < 1000000; i++) {
    ;
  }

  if (pthread_mutex_trylock(&flag_mutex) == 0) {
    printf("Thread %d é o vencedor da corrida!\n", thread_id);
  }

}

int main() {
  pthread_t thread[NUM_THREADS];
  int *threadid[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    threadid[i] = (int *)malloc(sizeof(int));
    *threadid[i] = i;
    pthread_create(&thread[i], NULL, contar, (void *)threadid[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
    free(threadid[i]);
  }

  pthread_mutex_destroy(&flag_mutex);
  
  pthread_exit(NULL);
}