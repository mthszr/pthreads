#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int N;
int **matriz;
int valor = 1; // Informa valor verdade para validar o jogo

void *checkMatrix(void *tid) {   
  int id = *((int *)tid);
  int linha[N], coluna[N];
  int i, j;

  for (i = 0; i < N; i++) {    // Vetor linha e coluna recebem valores de 1 a N
    linha[i] = i + 1;
    coluna[i] = i + 1;
  }
  for (i = 0; i < N; i++) {    // Thread verificará a linha e coluna correspondente ao seu id
    for (j = 0; j < N; j++) {    // Zera valor em vetor se igual ao da linha/coluna da matriz
      if (matriz[id][i] == linha[j]) { linha[j] = 0; }
      if (matriz[i][id] == coluna[j]) { coluna[j] = 0; }
    }
  }
  for (i = 0; i < N && valor == 1; i++) {    // Verifica se algum valor não é zero e invalida jogo
    if (linha[i] != 0) { valor = 0; }
    if (coluna[i] != 0) { valor = 0; }
  }
  pthread_exit(NULL);
}

void createMatrix(FILE *p) {
  int i, j, k;
  // Faz a alocação da matriz na memória
  matriz = (int **) malloc(N * sizeof(int *));
  if (matriz == NULL) { printf("Erro; matriz malloc\n"); exit(1); }
  for (i = 0; i < N; i++) { 
    matriz[i] = (int *) malloc(N * sizeof(int));
    if (matriz[i] == NULL) { printf("Erro; matriz malloc\n"); exit(1); }
  }
  // Lê o arquivo e atribui os valores na matriz
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      if (fscanf(p, "%d", &k) == 1) {
        matriz[i][j] = k;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *p = fopen("matriz.txt", "r");

  if (p == NULL) {
    printf("Erro ao abrir arquivo.\n");
  } else {
    if (fscanf(p, "%d", &N) == 1) {
      createMatrix(p);
      fclose(p);
      pthread_t threads[N];
      int *ids[N];
      int t, rc;
  
      for (t = 0; t < N; t++) {
        ids[t] = (int *) malloc(sizeof(int));
        if(ids[t] == NULL) { printf("ERRO; id malloc\n"); exit(1); }
        *ids[t] = t;
        // Threads verificarão uma linha e coluna da matriz
        rc = pthread_create(&threads[t], NULL, checkMatrix, (void *) ids[t]);
        if (rc) { printf("Erro; código de retorno %d\n", rc); exit(-1); }
      }
  
      for (t = 0; t < N; t++) {
        pthread_join(threads[t], NULL);
        free(ids[t]);
      }
      for (t = 0; t < N; t++) {
        free(matriz[t]);
      }
      free(matriz);
  
      if (valor) { printf("O jogo é válido!\n"); }
      else { printf("O jogo é inválido!\n"); }
    }
  }
  pthread_exit(NULL);
}