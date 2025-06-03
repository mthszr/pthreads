#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// variáveis globais
int num_candidatos;
int *votos_por_candidato; // vetor que armazena a contagem de votos por candidato
pthread_mutex_t *mutex_votos_candidato; // vetor de mutexes, um para cada candidato

// estrutura para passar argumentos para a thread
typedef struct {
  int id_arquivo;
} thread_arg_t;

// função para contar votos de um arquivo
void *processa_arquivo(void *arg) {
  thread_arg_t *dados_thread = (thread_arg_t *)arg;
  int id_arquivo = dados_thread->id_arquivo;

  char nome_arquivo[FILENAME_MAX];

  sprintf(nome_arquivo, "%d.in", id_arquivo);
  FILE *arquivo = fopen(nome_arquivo, "r");

  if (arquivo == NULL) {
    printf("Erro ao abrir o arquivo %s\n", nome_arquivo);
    pthread_exit(NULL);
  }

  int voto;
  while (fscanf(arquivo, "%d", &voto) == 1) {
    if (voto >= 0 && voto <= num_candidatos) {
      // atualiza a contagem de votos para o candidato
      pthread_mutex_lock(&mutex_votos_candidato[voto]);
      votos_por_candidato[voto]++;
      pthread_mutex_unlock(&mutex_votos_candidato[voto]);
    } else {
      printf("ATENÇÃO: Voto inválido %d encontrado no arquivo %s e ignorado\n", voto, nome_arquivo);
    }
  }
  fclose(arquivo);
  pthread_exit(NULL);
}

int imprimir_resultados() {
  int total_votos = 0;
  for (int i = 0; i <= num_candidatos; i++) {
    total_votos += votos_por_candidato[i];
  }

  printf("Total de votos computados: %d\n", total_votos);
  printf("\nResultado da votação:\n");

  printf("- Votos em branco: %d (%.2f%%)\n", votos_por_candidato[0], (votos_por_candidato[0] * 100.0) / total_votos);

  for (int i = 1; i <= num_candidatos; i++) {
    printf("- Candidato %d: %d votos (%.2f%%)\n", i, votos_por_candidato[i], (votos_por_candidato[i] * 100.0) / total_votos);
  }

  printf("\n");

  int id_candidato_vencedor = 1;
  int max_votos = votos_por_candidato[1];
  for (int i = 2; i <= num_candidatos; i++) {
    if (votos_por_candidato[i] > max_votos) {
      max_votos = votos_por_candidato[i];
      id_candidato_vencedor = i;
    }
  }

  printf("Candidato %d é o vencedor com %d votos (%.2f%%)\n", id_candidato_vencedor, max_votos, (max_votos * 100.0) / total_votos);

  return 0;
}

int main () {
  int num_arquivos;
  int num_threads;

  printf("Insira o n° de arquivos que serão lidos: "); // máximo de 5
  scanf("%d", &num_arquivos);

  printf("Insira o n° de threads (deve ser o mesmo de arquivos para esta versão): "); // deve ser igual ao número de arquivos
  scanf("%d", &num_threads);

  printf("Insira o n° de candidatos que participam da eleição: "); // máximo de 10
  scanf("%d", &num_candidatos);

  votos_por_candidato = calloc(num_candidatos + 1, sizeof(int));

  mutex_votos_candidato = malloc((num_candidatos + 1) * sizeof(pthread_mutex_t));

  for (int i = 0; i <= num_candidatos; i++) {
    pthread_mutex_init(&mutex_votos_candidato[i], NULL);
  }

  // aloca memória para as threads
  pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

  // aloca memória para os argumentos das threads
  thread_arg_t *args_threads = malloc(num_threads * sizeof(thread_arg_t));

  // cria as threads para processar os arquivos
  for (int i = 0; i < num_threads; i++) {
    args_threads[i].id_arquivo = i + 1;  // arquivos começam em 1
    pthread_create(&threads[i], NULL, processa_arquivo, (void *)&args_threads[i]);
  }

  imprimir_resultados();

  pthread_exit(NULL);
} 