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
#define MAX_NODES 10000       // Defina um limite máximo para os nós
#define NUM_THREADS 4         // Defina o número de threads usadas

int D[MAX_NODES];            // Distâncias mínimas
bool S[MAX_NODES];           // Nós visitados
int predecessor[MAX_NODES];   // Predecessores para reconstruir caminho
int n;
int graph[MAX_NODES][MAX_NODES]; // Matriz de adjacência
int minGraph[MAX_NODES][MAX_NODES]; // Matriz de adjacência para o grafo mínimo

// Mutex, semáforo e barreira para sincronização
pthread_mutex_t mutexD;
pthread_mutex_t mutexS;
sem_t semMinNode;
pthread_barrier_t barrier;

// Variáveis globais para armazenar o nó mínimo e a distância
int globalMinDist;
int globalMinNode;

struct ThreadArgs {
    int id;
};

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
            fscanf(file, "%d", &graph[i][j]);
        }
    }

    fclose(file);
}

// Função que cada thread executa para realizar o relaxamento das distâncias
void* dijkstra_thread(void* args) {
    int threadId = ((struct ThreadArgs*)args)->id;

    for (int iter = 0; iter < n; iter++) {
        int localMinDist = INFINITY;
        int localMinNode = -1;

        // Cada thread procura o nó não visitado com menor distância em seu subconjunto
        for (int i = threadId; i < n; i += NUM_THREADS) {
            pthread_mutex_lock(&mutexS);
            if (!S[i] && D[i] < localMinDist) {
                localMinDist = D[i];
                localMinNode = i;
            }
            pthread_mutex_unlock(&mutexS);
        }

        // Usamos um semáforo para atualizar a distância mínima global entre as threads
        sem_wait(&semMinNode);
        if (localMinDist < globalMinDist) {
            globalMinDist = localMinDist;
            globalMinNode = localMinNode;
        }
        sem_post(&semMinNode);

        pthread_barrier_wait(&barrier);

        // A thread mestre (id 0) marca o nó de menor distância global como visitado
        if (threadId == 0) {
            if (globalMinNode == -1) {
                printf("Nenhum nó acessível restante. Finalizando.\n");
                globalMinDist = INFINITY;
                pthread_barrier_wait(&barrier);
                break;
            }
            printf("Iteração %d: nó mínimo global: %d, distância: %d\n", iter, globalMinNode, globalMinDist);
            S[globalMinNode] = true;
        }

        pthread_barrier_wait(&barrier);

        if (globalMinNode == -1) break;

        // Cada thread atualiza as distâncias para os nós adjacentes ao nó global mínimo
        for (int i = threadId; i < n; i += NUM_THREADS) {
            pthread_mutex_lock(&mutexS);
            if (!S[i] && graph[globalMinNode][i] != INFINITY) {
                int newDist = D[globalMinNode] + graph[globalMinNode][i];
                pthread_mutex_lock(&mutexD);
                if (newDist < D[i]) {
                    D[i] = newDist;
                    predecessor[i] = globalMinNode;  // Atualiza o predecessor para construir o caminho mínimo
                    printf("Thread %d: atualizando distância do nó %d para %d (predecessor: %d)\n", threadId, i, newDist, globalMinNode);
                }
                pthread_mutex_unlock(&mutexD);
            }
            pthread_mutex_unlock(&mutexS);
        }

        pthread_barrier_wait(&barrier);

        // A thread mestre redefine as variáveis globais para a próxima iteração
        if (threadId == 0) {
            globalMinDist = INFINITY;
            globalMinNode = -1;
        }
    }

    pthread_exit(NULL);
}

// Função para imprimir o caminho mínimo até um destino
void print_path(int startNode, int endNode) {
    if (endNode == startNode) {
        printf("%d ", startNode);
    } else if (predecessor[endNode] == -1) {
        printf("Não há caminho de %d até %d\n", startNode, endNode);
    } else {
        print_path(startNode, predecessor[endNode]);
        printf("%d ", endNode);
    }
}

// Função para gerar o grafo mínimo baseado nos predecessores
void generate_min_graph() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            minGraph[i][j] = 0; // Inicializa com 0
        }
    }

    for (int i = 0; i < n; i++) {
        if (predecessor[i] != -1) {
            int pred = predecessor[i];
            minGraph[pred][i] = graph[pred][i];
            minGraph[i][pred] = graph[i][pred];  // Considerando que o grafo é não-direcionado
        }
    }
}

// Função para salvar a matriz de adjacência do grafo mínimo em um arquivo
void save_min_graph_to_file(char* filename) {
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

    globalMinDist = INFINITY;
    globalMinNode = -1;

    // Inicializa as distâncias e os predecessores
    for (int i = 0; i < n; i++) {
        D[i] = INFINITY;
        S[i] = false;
        predecessor[i] = -1;
    }

    int startNode = 0;
    D[startNode] = 0;

    // Inicializa semáforo, mutexes e barreira
    sem_init(&semMinNode, 0, 1);
    pthread_mutex_init(&mutexD, NULL);
    pthread_mutex_init(&mutexS, NULL);
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    // Cria as threads e inicia a execução do algoritmo
    pthread_t threads[NUM_THREADS];
    struct ThreadArgs threadArgs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threadArgs[i].id = i;
        pthread_create(&threads[i], NULL, dijkstra_thread, (void*)&threadArgs[i]);
    }

    // Aguarda o término das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Libera os recursos usados
    sem_destroy(&semMinNode);
    pthread_mutex_destroy(&mutexD);
    pthread_mutex_destroy(&mutexS);
    pthread_barrier_destroy(&barrier);

    // Exibe as distâncias mínimas e os caminhos
    printf("Distâncias mínimas do nó %d:\n", startNode);
    for (int i = 0; i < n; i++) {
        printf("Distância até o nó %d: %d | Caminho: ", i, D[i]);
        print_path(startNode, i);
        printf("\n");
    }

    // Gera o grafo mínimo e salva a matriz de adjacência em um arquivo
    generate_min_graph();
    save_min_graph_to_file(argv[2]);

    return 0;
}
