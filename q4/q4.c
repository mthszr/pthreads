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

int inicializar_recursos_contagem()  {
  votos_por_candidato = calloc(num_candidatos + 1, sizeof(int));
  if (votos_por_candidato == NULL) {
    printf("Erro ao alocar memória para votos_por_candidato\n");
    exit(-1);
  }

  mutex_votos_candidato = malloc((num_candidatos + 1) * sizeof(pthread_mutex_t));
  if (mutex_votos_candidato == NULL) {
    printf("Erro ao alocar memória para mutex_votos_candidato\n");
    free(votos_por_candidato);
    exit(-1);
  }

  for (int i = 0; i <= num_candidatos; i++) {
    if (pthread_mutex_init(&mutex_votos_candidato[i], NULL) != 0) {
      printf("Erro ao inicializar mutex para o candidato %d\n", i);
      free(votos_por_candidato);
      free(mutex_votos_candidato);
      exit(-1);
    }
  }
}

int gerenciar_threads(int num_threads, pthread_t **ptr_threads, thread_arg_t **ptr_args_threads ) {

  // aloca memória para as threads
  *ptr_threads = malloc(num_threads * sizeof(pthread_t));
  if (*ptr_threads == NULL) {
    printf("Erro ao alocar memória para threads\n");
    return 1;
  }

  // aloca memória para os argumentos das threads
  *ptr_args_threads = malloc(num_threads * sizeof(thread_arg_t));
  if (*ptr_args_threads == NULL) {
    printf("Erro ao alocar memória para args_threads\n");
    return 1;
  }

  // cria as threads para processar os arquivos
  for (int i = 0; i < num_threads; i++) {
    (*ptr_args_threads)[i].id_arquivo = i + 1;  // arquivos começam em 1
    if (pthread_create(&(*ptr_threads)[i], NULL, processa_arquivo, (void *)&(*ptr_args_threads)[i]) != 0) {
      printf("Erro ao criar a thread para o arquivo %d\n", i + 1);
      
      for (int j = 0; j < i; ++j) {
        pthread_join((*ptr_threads)[j], NULL);
      }
       
      return 1;
    }
  }

  for (int i = 0; i < num_threads; i++) {
    pthread_join((*ptr_threads)[i], NULL);
  }

  return 0;
}

int imprimir_resultados() {
  int total_votos = 0;
  for (int i = 0; i <= num_candidatos; i++) {
    total_votos += votos_por_candidato[i];
  }

  printf("Total de votos computados: %d\n", total_votos);
  printf("\nResultado da votação:\n");

  printf("- Votos em branco: %d (%.2f%%)\n",
        votos_por_candidato[0],
        (votos_por_candidato[0] * 100.0) / total_votos);

  for (int i = 1; i <= num_candidatos; i++) {
    printf("- Candidato %d: %d votos (%.2f%%)\n", i,
          votos_por_candidato[i],
          (votos_por_candidato[i] * 100.0) / total_votos);
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

  printf("Candidato %d é o vencedor com %d votos (%.2f%%)\n",
        id_candidato_vencedor,
        max_votos,
        (max_votos * 100.0) / total_votos);

  return 0;
}

int liberar_recursos(pthread_t *threads, thread_arg_t *args_threads) {
  if (threads != NULL) {
    free(threads);
  }

  if (args_threads != NULL) {
    free(args_threads);
  }

  if (mutex_votos_candidato != NULL) {
    for (int i = 0; i <= num_candidatos; i++) {
      pthread_mutex_destroy(&mutex_votos_candidato[i]);
    }
    free(mutex_votos_candidato);
  }

  if (votos_por_candidato != NULL) {
    free(votos_por_candidato);
  }
  
  return 0;
}

int main () {
  int num_arquivos;
  int num_threads;
  pthread_t *threads = NULL;
  thread_arg_t *args_threads = NULL;

  printf("Insira o n° de arquivos que serão lidos: "); // máximo de 5
  scanf("%d", &num_arquivos);

  printf("Insira o n° de threads: "); // deve ser igual ao número de arquivos
  scanf("%d", &num_threads);

  printf("Insira o n° de candidatos que participam da eleição: "); // máximo de 10
  scanf("%d", &num_candidatos);

  inicializar_recursos_contagem();

  gerenciar_threads(num_threads, &threads, &args_threads);

  imprimir_resultados();

  liberar_recursos(threads, args_threads);

  pthread_exit(NULL);
}