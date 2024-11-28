#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

// Função para encontrar o nó com a menor distância não visitado
int minDistance(int dist[], int sptSet[], int n) {
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < n; v++) {
        if (!sptSet[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }

    return min_index;
}

// Função que implementa o algoritmo de Dijkstra
void dijkstra(int **graph, int n, int src, int dist[], int parent[]) {
    int *sptSet = (int *)malloc(n * sizeof(int)); // Conjunto de vértices para os quais o caminho mais curto é conhecido

    // Inicializa todos os valores
    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = 0;
        parent[i] = -1;
    }

    dist[src] = 0;

    // Calcula o caminho mais curto para todos os nós
    for (int count = 0; count < n - 1; count++) {
        int u = minDistance(dist, sptSet, n);

        sptSet[u] = 1;

        for (int v = 0; v < n; v++) {
            if (!sptSet[v] && graph[u][v] != 0 && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                parent[v] = u;
            }
        }
    }

    free(sptSet);
}

// Função para salvar a matriz de adjacências no arquivo
void saveMatrixToFile(char *filename, int **matrix, int n) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para salvar a matriz!\n");
        return;
    }

    fprintf(file, "%d\n", n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Função principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Inicia o contador de tempo
    clock_t start_time = clock();

    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("Erro ao abrir o arquivo de entrada!\n");
        return 1;
    }

    int n;
    fscanf(inputFile, "%d", &n); // Lê o número de nós

    // Aloca memória para a matriz de adjacências
    int **graph = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        graph[i] = (int *)malloc(n * sizeof(int));
    }

    // Lê a matriz de adjacências
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fscanf(inputFile, "%d", &graph[i][j]);
        }
    }

    fclose(inputFile);

    int *dist = (int *)malloc(n * sizeof(int));
    int *parent = (int *)malloc(n * sizeof(int));

    // Suponha que o nó de origem seja 0
    int src = 0;
    dijkstra(graph, n, src, dist, parent);

    // Cria a matriz de adjacências do caminho mínimo
    int **shortestPathMatrix = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        shortestPathMatrix[i] = (int *)malloc(n * sizeof(int));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                shortestPathMatrix[i][j] = 0;
            } else if (parent[j] == i) {
                shortestPathMatrix[i][j] = graph[i][j];
            } else {
                shortestPathMatrix[i][j] = 0;
            }
        }
    }

    // Salva a matriz de adjacências do caminho mínimo no arquivo de saída
    saveMatrixToFile(argv[2], shortestPathMatrix, n);

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Execução sequencial finalizada. Tempo de execução: %f\n", time_taken);

    // Libera a memória alocada
    for (int i = 0; i < n; i++) {
        free(graph[i]);
        free(shortestPathMatrix[i]);
    }
    free(graph);
    free(shortestPathMatrix);
    free(dist);
    free(parent);

    return 0;
}