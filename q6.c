#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS_LEITORAS 20
#define THREADS_ESCRITORAS 20
#define TAMANHO_BD 100

// banco de dados para simular leituras e escritas
int BD[TAMANHO_BD];

// variáveis de estado
int leitores_ativos = 0;    
int escrevendo = 0; // deve ser 0 ou 1
int leitores_esperando = 0;   
int escritores_esperando = 0;  

// protege o acesso às variáveis de estado
pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pode_ler = PTHREAD_COND_INITIALIZER;
pthread_cond_t pode_escrever = PTHREAD_COND_INITIALIZER;

void sleep() {
  int i;
  for (i = 0; i < 200000000; i++) {
    ; 
  }
}

void *ler(void *threadid) {
  int thread_id = *((int *)threadid);

  while (1) {
    pthread_mutex_lock(&mutex_estado);

    leitores_esperando++;  // indica que um leitor está tentando ler

    // um leitor deve esperar se:
    // há um escritor ativo (escrevendo)
    // ou se há escritores na fila de espera (escritores_esperando > 0)
    // esta condição ajuda a prevenir starvation de escritores
    while (escrevendo || escritores_esperando > 0) {
      pthread_cond_wait(&pode_ler, &mutex_estado);
    }

    leitores_esperando--; // não está mais esperando
    leitores_ativos++; // está ativo como leitor

    pthread_mutex_unlock(&mutex_estado);

    // leitura ocorre fora do bloqueio do mutex de estado para permitir leituras concorrentes
    int posicao = rand() % TAMANHO_BD;
    int dado = BD[posicao];
    printf("Leitor %d leu o dado %d da posição %d\n", thread_id, dado, posicao);

    pthread_mutex_lock(&mutex_estado);
    leitores_ativos--; // leitor terminou sua leitura

    // se este foi o último leitor e há escritores esperando
    if (leitores_ativos == 0 && escritores_esperando > 0) {
      pthread_cond_broadcast(&pode_escrever); // acorda escritores esperando
    }

    pthread_mutex_unlock(&mutex_estado);

    sleep();
  }
}

void *escrever(void *threadid) {
  int thread_id = *((int *)threadid);

  while (1) {
    int posicao = rand() % TAMANHO_BD;
    int data = thread_id;

    pthread_mutex_lock(&mutex_estado);

    escritores_esperando++; // indica que um escritor está tentando escrever

    // um escritor deve esperar se:
    // há leitores ativos (leitores_ativos > 0)
    // ou se há outro escritor ativo (escrevendo)
    while (leitores_ativos > 0 || escrevendo) {
        pthread_cond_wait(&pode_escrever, &mutex_estado);
    }

    escritores_esperando--; // não esta mais esperando
    escrevendo = 1; // está ativo como escritor

    pthread_mutex_unlock(&mutex_estado);

    // 'escrevendo' garante exclusividade
    BD[posicao] = data;
    printf("Escritor %d escreveu o dado %d na posição %d\n", thread_id, data, posicao);

    pthread_mutex_lock(&mutex_estado);
    escrevendo = 0; // escritor terminou sua escrita

    // decide quem acordar:
    // se há escritores esperando, acorda eles
    // senão, acorda leitores esperando
    if (escritores_esperando > 0) {
      pthread_cond_broadcast(&pode_escrever); 
    } else {
      pthread_cond_broadcast(&pode_ler); 
    }

    pthread_mutex_unlock(&mutex_estado);

    sleep();
  }
}

int main() {
  srand(time(NULL));

  pthread_t thread_leitora[THREADS_LEITORAS];
  pthread_t thread_escritora[THREADS_ESCRITORAS];
  int *threadid_leitora[THREADS_LEITORAS];
  int *threadid_escritora[THREADS_ESCRITORAS];

  // inicializa o banco de dados
  for (int i = 0; i < TAMANHO_BD; i++) {
    BD[i] = -1;
  }

  // inicializa as threads escritoras
  for (int i = 0; i < THREADS_ESCRITORAS; i++) {
    threadid_escritora[i] = (int *)malloc(sizeof(int));
    *threadid_escritora[i] = i;
    pthread_create(&thread_escritora[i], NULL, escrever, (void *)threadid_escritora[i]);
  }

  // inicializa as threads leitoras
  for (int i = 0; i < THREADS_LEITORAS; i++) {
    threadid_leitora[i] = (int *)malloc(sizeof(int));
    *threadid_leitora[i] = i;
    pthread_create(&thread_leitora[i], NULL, ler, (void *)threadid_leitora[i]);  
  }

  for (int i = 0; i < THREADS_LEITORAS; i++) {
    pthread_join(thread_leitora[i], NULL);
    free(threadid_leitora[i]);
  }

  for (int i = 0; i < THREADS_ESCRITORAS; i++) {
    pthread_join(thread_escritora[i], NULL);
    free(threadid_escritora[i]);
  }

  pthread_mutex_destroy(&mutex_estado);
  pthread_cond_destroy(&pode_ler);
  pthread_cond_destroy(&pode_escrever);

  pthread_exit(NULL);
}