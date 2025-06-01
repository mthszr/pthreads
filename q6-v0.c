#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADS_LEITORAS 5
#define THREADS_ESCRITORAS 10
#define TAMANHO_BD 100

int BD[TAMANHO_BD];
int contador_leitores = 0;
int escrevendo = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pode_escrever = PTHREAD_COND_INITIALIZER;
pthread_cond_t pode_ler = PTHREAD_COND_INITIALIZER;

void sleep() {
    int i;
    for(i = 0; i < 1000000000; i++) {
        ;  
    }
}

void *ler(void *threadid) {
    int thread_id = *((int *)threadid);

    while (1) {
        pthread_mutex_lock(&mutex);

        while (escrevendo) {
            pthread_cond_wait(&pode_ler, &mutex);
        }

        contador_leitores++;
        pthread_mutex_unlock(&mutex);

        int posicao = rand() % TAMANHO_BD;  // escolhe uma posição aleatória do banco de dados para ler
        int dado = BD[posicao];
        printf("Leitor %d leu o dado %d da posição %d\n", thread_id, dado, posicao);

        sleep();  // simula o tempo de leitura

        pthread_mutex_lock(&mutex);
        contador_leitores--;
        if (contador_leitores == 0) {
            pthread_cond_broadcast(&pode_escrever);  // sinaliza que não há mais leitores
        }
        pthread_mutex_unlock(&mutex);
    }

}

void *escrever(void *threadid) {
    int thread_id = *((int *)threadid);

    while (1) {
        pthread_mutex_lock(&mutex);

        while (contador_leitores > 0 || escrevendo) {
            pthread_cond_wait(&pode_escrever, &mutex);
        }

        escrevendo = 1;

        pthread_mutex_unlock(&mutex);

        int posicao = rand() % TAMANHO_BD;
        BD[posicao] = thread_id;
        printf("Escritor %d escreveu na posição %d\n", thread_id, posicao);

        //sleep();

        pthread_mutex_lock(&mutex);
        escrevendo = 0;
        pthread_cond_broadcast(&pode_ler);
        pthread_cond_broadcast(&pode_escrever);  // sinaliza que a escrita terminou

        pthread_mutex_unlock(&mutex);
    }

}

int main() {
    pthread_t thread_leitora[THREADS_LEITORAS];
    pthread_t thread_escritora[THREADS_ESCRITORAS];
    int *threadid_leitora[THREADS_LEITORAS];
    int *threadid_escritora[THREADS_ESCRITORAS];

    int return_code;
    int i;

    // inicializa o banco de dados
    for (i = 0; i < TAMANHO_BD; i++) {
        BD[i] = -1;  // indica que a posição está vazia
    }


    // inicializa as threads escritoras
    for (i = 0; i < THREADS_ESCRITORAS; i++) {
        threadid_escritora[i] = (int *)malloc(sizeof(int));
        *threadid_escritora[i] = i;

        return_code = pthread_create(&thread_escritora[i], NULL, escrever, (void *)threadid_escritora[i]);

        if (return_code) {
            printf("Erro ao criar a thread escritora %d: %d\n", i, return_code);
            exit(-1);
        }
    }

    // inicializa as threads leitoras
    for (i = 0; i < THREADS_LEITORAS; i++) {
        threadid_leitora[i] = (int *)malloc(sizeof(int));
        *threadid_leitora[i] = i;

        return_code = pthread_create(&thread_leitora[i], NULL, ler, (void *)threadid_leitora[i]);

        if (return_code) {
            printf("Erro ao criar a thread leitora %d: %d\n", i, return_code);
            exit(-1);
        }
    }

    for (i = 0; i < THREADS_LEITORAS; i++) {
        pthread_join(thread_leitora[i], NULL);
        free(threadid_leitora[i]); 
    }

    for (i = 0; i < THREADS_ESCRITORAS; i++) {
        pthread_join(thread_escritora[i], NULL);
        free(threadid_escritora[i]);  
    }

    pthread_cond_destroy(&pode_escrever);
    pthread_cond_destroy(&pode_ler);

    pthread_exit(NULL);
}