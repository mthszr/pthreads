#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>

#define N_SEATS 30
#define STUDENTS 300

int turn = 0; // Informa o turno para alternar o uso

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t change = PTHREAD_COND_INITIALIZER; // Sinaliza a mudança da thread

void *use_a(); // Funções para uso da sala por curso
void *use_b();

int main(int argc, char *argv[]) {
  pthread_t course_a; 
  pthread_t course_b; 
  pthread_create(&course_a, NULL, use_a, NULL);
  pthread_create(&course_b, NULL, use_b, NULL);
  pthread_join(course_a, NULL);  
  pthread_join(course_b, NULL);
  pthread_exit(NULL);
}

void *use_a() {
  int i;
  int u = (STUDENTS / N_SEATS); // Número de usos da sala para atender estudantes do curso
  for (i = 1; i <= u; i++) {
    pthread_mutex_lock(&mutex);
    while (turn) {    // Se turno for 1, irá esperar sinal de mudança e turno 0
      pthread_cond_wait(&change, &mutex);
    }
    printf("Curso A usou a sala %d vez(es)\n", i);
    turn = !turn;
    pthread_cond_signal(&change);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
}

void *use_b() {
  int i;
  int u = (STUDENTS / N_SEATS);
  for (i = 1; i <= u; i++) {
    pthread_mutex_lock(&mutex);
    while (!turn) {    // Se turno for 0, irá esperar sinal de mudança e turno 1
      pthread_cond_wait(&change, &mutex);
    }
    printf("Curso B usou a sala %d vez(es)\n", i);
    turn = !turn;
    pthread_cond_signal(&change);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
}