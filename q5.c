#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define VOLTAS 2
#define THREADS 10
#define INTERSECAO 5
#define TEMPO 500000
// Tempo (500 milissegundos) está em microssegundos
int intersection[INTERSECAO]; // Informa o número de trens por interseção

pthread_mutex_t mutex[INTERSECAO];
pthread_cond_t empty[INTERSECAO]; // Sinaliza algum espaço vazio por interseção

void *move(void *tid);

int main(int argc, char *argv[]) {
  pthread_t threads[THREADS];
  int *ids[THREADS];
  int t, rc;

  for (t = 0; t < INTERSECAO; t++) {
    intersection[t] = 0;    // Atribui número 0 (trens) para cada interseção
    pthread_mutex_init(&mutex[t], NULL);
    pthread_cond_init(&empty[t], NULL);
  }

  for (t = 0; t < THREADS; t++) {  
    ids[t] = (int *) malloc(sizeof(int));
    if(ids[t] == NULL) { printf("ERRO; id malloc\n"); exit(1); }
    *ids[t] = t;

    rc = pthread_create(&threads[t], NULL, move, (void *) ids[t]);
    if (rc) {
      printf("ERRO; código de retorno %d\n", rc);
      exit(-1);
    }
  }

  for (t = 0; t < THREADS; t++) {
    pthread_join(threads[t], NULL);
    free(ids[t]);
  }

  for (t = 0; t < INTERSECAO; t++) {
    pthread_mutex_destroy(&mutex[t]);
    pthread_cond_destroy(&empty[t]);
  }
  pthread_exit(NULL);
}

void *move(void *tid) {
  int id = *((int *) tid);
  int i, v;

  for (v = 0; v < VOLTAS; v++) {
    for (i = 0; i < INTERSECAO; i++) {
      pthread_mutex_lock(&mutex[i]);   // Acesso a interseção é feito com o mutex
      while (intersection[i] == 2) {    // Se interseção está ocupada, espera sinal
        pthread_cond_wait(&empty[i], &mutex[i]);
      }
      printf("Trem %d entrou na intersecão %d\n", id + 1, i + 1);
      intersection[i]++;
      pthread_mutex_unlock(&mutex[i]);    // Depois de ocupar a interseção, libera o mutex
      // Depois do tempo decorrer e desocupar, sinaliza as threads em espera na interseção i
      usleep(TEMPO);
      printf("Trem %d saiu da intersecão %d\n", id + 1, i + 1);
      intersection[i]--;
      pthread_cond_broadcast(&empty[i]);
    }
  }
  pthread_exit(NULL);
}