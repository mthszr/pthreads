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

    int return_code;
    int i;

    for (i = 0; i < NUM_THREADS; i++) {
        threadid[i] = (int *)malloc(sizeof(int));
        *threadid[i] = i;
        return_code = pthread_create(&thread[i], NULL, contar, (void *)threadid[i]);

        if (return_code) {
            printf("Error creating thread %d: %d\n", i, return_code);
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
      pthread_join(thread[i], NULL);
    }

    for (i = 0; i < NUM_THREADS; i++) {
      free(threadid[i]);
    }

    pthread_exit(NULL);
}