/*
 * Autores: Bruna Falheiro e Júlio Bularmaqui
 *
 * Implementação concorrente do algoritmo de Dijkstra utilizando threads.
 * O objetivo é calcular as distâncias mínimas de um nó inicial para todos os outros em um grafo,
 * utilizando múltiplas threads.
 * 
 * Parâmetros:
 * - argv[1]: Arquivo de entrada contendo a matriz de adjacência do grafo.
 * - argv[2]: Arquivo de saída para armazenar a matriz de caminhos mínimos.
 * - argv[3]: Número de threads a serem utilizadas.
 * 
 * A execução ocorre da seguinte forma:
 * 1. O grafo é lido do arquivo e armazenado na matriz de adjacência `D`.
 * 2. Inicializa-se o vetor `S` para armazenar as distâncias mínimas de cada nó, com `S[0]` inicializado como 0.
 * 3. Cada thread realiza o relaxamento das distâncias para os nós ainda não visitados. O
 *    relaxamento de uma aresta é feito comparando a distância atual de um nó v com a distância
 *    mínima calculada passando por um nó u já processado. Se a distância de v via u for menor,
 *    a distância de v é atualizada.
 * 4. Após a execução, a matriz de caminhos mínimos é gerada e salva no arquivo de saída.
 */

#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdatomic.h>

#define INFINITY INT_MAX

int **D; // Matriz de distâncias mínimas entre os nós
int *S; // Vetor de distâncias mínimas do nó inicial para todos os outros nós
int *predecessor; // Vetor de predecessores de cada nó, utilizado para reconstruir o caminho
int *minGraph; // Vetor que marca os nós já processados (com a menor distância)
int n; // Número de nós no grafo
int u; // Nó com a menor distância
int NUM_THREADS;

pthread_barrier_t barrier;
pthread_mutex_t mutex;

// Função para ler o grafo a partir do arquivo de entrada
void read_graph_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }

    // Lê o número de nós
    fscanf(file, "%d", &n); 

    D = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        D[i] = (int*)malloc(n * sizeof(int));
    }
    S = (int*)malloc(n * sizeof(int));
    predecessor = (int*)malloc(n * sizeof(int));
    minGraph = (int*)malloc(n * sizeof(int));

    // Lê a matriz de adjacências do arquivo
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fscanf(file, "%d", &D[i][j]);
        }
    }

    fclose(file);
}

// Função para encontrar o nó com a menor distância não visitado
int minDistance() {
    int min = INFINITY, min_index = -1;

    // Percorre todos os nós e encontra o nó com a menor distância ainda não processado
    for (int v = 0; v < n; v++) {
        // Verifica se o nó não foi processado e se a distância é a menor
        if (!minGraph[v] && S[v] <= min) {
            min = S[v];
            min_index = v;
        }
    }

    return min_index;
}

void* dijkstra(void* arg) {
    int thread_id = *(int*)arg;
    int local_u;

    // Itera para encontrar a distância mínima de todos os nós
    for (int count = 0; count < n - 1; count++) {
        // Cada thread encontra localmente o nó com a menor distância e comunica a thread 0
        if (thread_id == 0) {
            local_u = minDistance(); // Encontra o nó com a menor distância
            if (local_u != -1) {
                atomic_store(&u, local_u); // Atualiza a variável atômica u com o nó encontrado
                minGraph[local_u] = 1; // Marca o nó como processado
            }
        }

        // Sincroniza todas as threads para garantir que a thread 0 atualizou a variável u
        pthread_barrier_wait(&barrier);

        // Cada thread carrega o valor atualizado de u
        pthread_mutex_lock(&mutex);
        local_u = u;
        pthread_mutex_unlock(&mutex);

        // Se não há mais nós a processar, as threads encerram
        if (local_u == -1) { break; }

        // Realiza o relaxamento das distâncias para os nós restantes
        for (int v = thread_id; v < n; v += NUM_THREADS) {
            if (!minGraph[v] && D[local_u][v] != 0 && S[local_u] != INFINITY && S[local_u] + D[local_u][v] < S[v]) {
                S[v] = S[local_u] + D[local_u][v]; // Atualiza a distância de v
                predecessor[v] = local_u; // Atualiza o predecessor de v
            }
        }

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

// Função para salvar a matriz de caminhos mínimos no arquivo de saída
void save_min_graph_to_file(char* filename, int **minGraph, int n) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para salvar!\n");
        exit(1);
    }

    // Escreve o número de nós no arquivo
    fprintf(file, "%d\n", n);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(file, "%d ", minGraph[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida> <num_threads>\n", argv[0]);
        return 1;
    }

    clock_t start_time = clock();
    NUM_THREADS = atoi(argv[3]); 

    read_graph_from_file(argv[1]);

    // Inicializa os vetores de distâncias e predecessores
    for (int i = 0; i < n; i++) {
        S[i] = INFINITY;
        minGraph[i] = 0;
        predecessor[i] = -1;
    }

    // Nó de origem = 0
    S[0] = 0;

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Cria as threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, dijkstra, &thread_ids[i]);
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    // Aloca memória para armazenar a matriz de caminhos mínimos
    int **shortestPathMatrix = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        shortestPathMatrix[i] = (int*)malloc(n * sizeof(int));
    }

    // Preenche a matriz de caminhos mínimos com base nos predecessores
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                shortestPathMatrix[i][j] = 0;
            } else if (predecessor[j] == i) {
                shortestPathMatrix[i][j] = D[i][j]; // Se o nó j é o próximo nó de i no caminho, atribui a distância direta
            } else {
                shortestPathMatrix[i][j] = 0; // Caso contrário, o caminho não é direto
            }
        }
    }

    save_min_graph_to_file(argv[2], shortestPathMatrix, n);

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Execução concorrente finalizada. Tempo de execução: %f\n\n", time_taken);

    for (int i = 0; i < n; i++) {
        free(D[i]);
        free(shortestPathMatrix[i]);
    }

    free(D);
    free(shortestPathMatrix);
    free(S);
    free(predecessor);
    free(minGraph);

    return 0;
}