#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int nThreads; // Tamanho do vetor a ser somadoe
long int vector_size; // Tamanho do vetor a sr somado == n
float *first_vector; // Vetor a ser somado == vet
float *second_vector; // Vetor a ser somado
float result_produto_interno; // Resultado do produto interno

void *produtoInterno(void *tid) {
  int id = (long int) tid; // Identificador da thread
  float *local_product = (float *) malloc(sizeof(float)); // Aloca memória para o produto local

  if (local_product == NULL) {
    printf("Erro ao alocar memória para local_product\n");
    pthread_exit(NULL);
  }

  *local_product = first_vector[id] * second_vector[id]; // Calcula o produto interno local
  // printa a thread, os valores dos vetores e o produto interno local
  printf("Thread %d: %f * %f = %f\n", id, first_vector[id], second_vector[id], *local_product);
  pthread_exit((void*) local_product); // Retorna o produto local
}

int main(int argc, char *argv[]) {
  FILE *file; // Arquivo de entrada  == arq
  size_t ret1; // Retorno da função de leitura no arquivo de entrada == ret
  size_t ret2; // Retorno da função de leitura no arquivo de entrada
  size_t retProdutoInterno; // Retorno da função de leitura no arquivo de entrada
  float expectedProdutoInterno; // Resultado do produto interno
  void *retorno_thread; // Auxiliar para retorno das threads

  printf("Thread principal\n");

  // Verifica se os três argumentos foram passados corretamente
  if (argc < 3) {
    printf("Use: %s <arquivo de entrada> <número de threads>\n", argv[0]);
    return 0;
  }

  // Abre o arquivo de entrada com os valores para serem somados
  file = fopen(argv[1], "rb");
  if (file == NULL) {
    printf("--ERRO: fopen()\n");
    return 0;
  }

  // Lê o tamanho do primeiro vetor (primeira linha do arquivo)
  ret1 = fread(&vector_size, sizeof(long int), 1, file);
  if (!ret1) {
    fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo\n");
    return 0;
  }

  // Aloca espaço de memória e carrega o vetor de entrada
  first_vector = malloc(sizeof(float) * vector_size);
  if (first_vector == NULL) {
    printf("--ERRO: malloc() 1\n");
    return 0;
  }

  // Lê o primeiro vetor
  ret1 = fread(first_vector, sizeof(float), vector_size, file);
  if (ret1 < vector_size) {
    fprintf(stderr, "Erro de leitura dos elementos do vetor 1\n");
    return 0;
  }

  // Aloca espaço de memória e carrega o segundo vetor de entrada
  second_vector = malloc(sizeof(float) * vector_size);
  if (second_vector == NULL) {
    printf("--ERRO: malloc() 2\n");
    return 0;
  }

  // Lê o segundo vetor
  ret2 = fread(second_vector, sizeof(float), vector_size, file);
  if (ret2 < vector_size) {
    fprintf(stderr, "Erro de leitura dos elementos do vetor 2\n");
    return 0;
  }

  // Lê o produto interno do arquivo
  retProdutoInterno = fread(&expectedProdutoInterno, sizeof(float), 1, file);
  if (!retProdutoInterno) {
    fprintf(stderr, "Erro de leitura do produto interno\n");
    return 0;
  }

  // Imprime os elementos do vetor 1
  printf("Elementos do vetor 1: ");
  for (int i = 0; i < vector_size; i++) {
    printf("%f ", first_vector[i]);
  }
  
  // Imprime os elementos do vetor 2
  printf("\nElementos do vetor 2: ");
  for (int i = 0; i < vector_size; i++) {
    printf("%f ", second_vector[i]);
  }

  // Imprime o produto interno
  printf("\nProduto interno: %f\n", expectedProdutoInterno);

  // Lê o número de threads da entrada do usuário
  nThreads = atoi(argv[2]);
  if (nThreads > vector_size) {
    nThreads = vector_size;
  }

  pthread_t *tid_sistema; // Vetor de identificadores das threads no sistema
  // Aloca espaço para o vetor de identificadores das threads no sistema
  tid_sistema = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);
  if (tid_sistema == NULL) {
    printf("--ERRO: malloc()\n");
    return 0;
  }

  // Inicializa o produto interno
  result_produto_interno = 0;

  // Cria as threads
  for (long int i = 0; i < nThreads; i++) {
    if (pthread_create(&tid_sistema[i], NULL, produtoInterno, (void*) i)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  for (int i = 0; i < nThreads; i++) {
    if (pthread_join(tid_sistema[i], &retorno_thread)) {
      printf("--ERRO: pthread_join()\n");
      return 0;
    }
    printf("Thread %d finalizada\n", i); 
    result_produto_interno += *(float *) retorno_thread;
  }

  printf("\n\n\n\nThread principal finalizada\n");
  printf("Produto interno esperado: %f\n", expectedProdutoInterno);
  printf("Produto interno calculado: %f\n", result_produto_interno);

  float variacao_relativa = (expectedProdutoInterno - result_produto_interno) / expectedProdutoInterno;
  printf("Variacao relativa: %f\n", variacao_relativa);

  // Fecha o arquivo e libera a memória alocada
  fclose(file);
  free(first_vector);
  free(second_vector);

  return 0;
}