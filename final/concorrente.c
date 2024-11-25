/*
 * Este programa implementa o algoritmo de Dijkstra para encontrar o caminho mínimo em um grafo
 * de forma concorrente, usando múltiplas threads. A tarefa é dividida entre as threads da
 * seguinte maneira:
 * 1. Cada thread examina um subconjunto de nós para encontrar o nó com a menor distância
 *    ainda não visitado (relaxamento).
 * 2. As threads atualizam a distância mínima globalmente usando uma barreira e um semáforo
 *    para sincronização.
 * 3. A thread principal escolhe o nó mínimo globalmente acessível e o marca como visitado.
 * 4. As threads atualizam as distâncias para os nós adjacentes, ajustando as distâncias com base
 *    nos valores mínimos encontrados e registrados na matriz de predecessores para reconstruir o caminho.
 */

#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INFINITY INT_MAX
#define MAX_NODES 100 // Defina um limite máximo para os nós
#define NUM_THREADS 4 // Defina o número de threads usadas

int D[MAX_NODES][MAX_NODES]; // Distâncias mínimas
int S[MAX_NODES];
int predecessor[MAX_NODES]; // Predecessores para reconstruir caminho
int minGraph[MAX_NODES];
int n;
int u; // Variável global para o nó com a menor distância

// Mutex, semáforo e barreira para sincronização
pthread_barrier_t barrier;
pthread_mutex_t mutex;

// Função para ler a matriz de adjacências do arquivo de entrada
void read_graph_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }

    fscanf(file, "%d", &n);  // Lê o número de nós
    if (n > MAX_NODES) {
        printf("Número de nós excede o limite máximo!\n");
        exit(1);
    }

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

    for (int v = 0; v < n; v++) {
        if (!minGraph[v] && S[v] <= min) {
            min = S[v];
            min_index = v;
        }
    }

    return min_index;
}

// Função que cada thread executa para realizar o relaxamento das distâncias
void* dijkstra_thread(void* arg) {
    int thread_id = *(int*)arg;

    for (int count = 0; count < n - 1; count++) {
        if (thread_id == 0) {
            pthread_mutex_lock(&mutex);
            u = minDistance();
            minGraph[u] = 1;
            pthread_mutex_unlock(&mutex);
        }

        pthread_barrier_wait(&barrier);

        for (int v = thread_id; v < n; v += NUM_THREADS) {
            if (!minGraph[v] && D[u][v] != 0 && S[u] != INFINITY && S[u] + D[u][v] < S[v]) {
                pthread_mutex_lock(&mutex);
                S[v] = S[u] + D[u][v];
                predecessor[v] = u;
                pthread_mutex_unlock(&mutex);
            }
        }

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

// Função para salvar a matriz de adjacência do grafo mínimo em um arquivo
void save_min_graph_to_file(char* filename, int minGraph[MAX_NODES][MAX_NODES], int n) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para salvar!\n");
        exit(1);
    }

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
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Lê a matriz de adjacências do arquivo de entrada
    read_graph_from_file(argv[1]);

    // Inicializa as distâncias e os predecessores
    for (int i = 0; i < n; i++) {
        S[i] = INFINITY;
        minGraph[i] = 0;
        predecessor[i] = -1;
    }

    S[0] = 0; // Suponha que o nó de origem seja 0

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    pthread_mutex_init(&mutex, NULL);

    // Cria as threads e inicia a execução do algoritmo
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, dijkstra_thread, &thread_ids[i]);
    }

    // Aguarda o término das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    // Cria a matriz de adjacências do caminho mínimo
    int shortestPathMatrix[MAX_NODES][MAX_NODES];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                shortestPathMatrix[i][j] = 0;
            } else if (predecessor[j] == i) {
                shortestPathMatrix[i][j] = D[i][j];
            } else {
                shortestPathMatrix[i][j] = 0;
            }
        }
    }

    // Salva a matriz de adjacências do caminho mínimo no arquivo de saída
    save_min_graph_to_file(argv[2], shortestPathMatrix, n);

    printf("Matriz de caminhos mínimos salva no arquivo '%s'.\n", argv[2]);

    return 0;
}