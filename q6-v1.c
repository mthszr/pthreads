#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS_LEITORAS 20
#define THREADS_ESCRITORAS 20
#define TAMANHO_BD 100

int BD[TAMANHO_BD];
int contador_leitores = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db = PTHREAD_MUTEX_INITIALIZER;

void sleep() {
    int i;
    for (i = 0; i < 200000000; i++) {
        ;
    }
}

void *ler(void *threadid) {
    int thread_id = *((int *)threadid);

    while (1) {
        pthread_mutex_lock(&mutex);

        contador_leitores += 1;
        if (contador_leitores == 1) {
            pthread_mutex_lock(&db);  // bloqueia o banco de dados para leitura
        }

        pthread_mutex_unlock(&mutex);

        int posicao = rand() % TAMANHO_BD;  // escolhe uma posição aleatória do banco de dados para ler
        int dado = BD[posicao];
        printf("Leitor %d leu o dado %d da posição %d\n", thread_id, dado, posicao);

        pthread_mutex_lock(&mutex);
        contador_leitores -= 1;
        if (contador_leitores == 0) {
            pthread_mutex_unlock(&db);  // libera o banco de dados para escrita
        }
        pthread_mutex_unlock(&mutex);

        sleep();
    }
}

void *escrever(void *threadid) {
    int thread_id = *((int *)threadid);

    while (1) {
        int posicao = rand() % TAMANHO_BD;  // escolhe uma posição aleatória do banco de dados para escrever
        int data = thread_id;                // dado a ser escrito

        pthread_mutex_lock(&db);  // bloqueia o banco de dados para escrita

        BD[posicao] = data;  // escreve o dado na posição escolhida
        printf("Escritor %d escreveu na posição %d\n", thread_id, posicao);
        pthread_mutex_unlock(&db);  // libera o banco de dados para leitura/escrita

        sleep();  // simula o tempo de escrita
    }
}

int main() {
    srand(time(NULL));  // gerador de números aleatórios

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

    pthread_exit(NULL);
}