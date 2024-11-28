#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PESO 20 // Peso máximo para as arestas

// Estrutura para um nó da lista encadeada
typedef struct Node {
    int vertice;
    int peso;
    struct Node* proximo;
} Node;

// Função para gerar um número aleatório dentro de um intervalo
int random_weight(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Função para adicionar uma aresta à lista de adjacência
void adicionar_aresta(Node** lista, int origem, int destino, int peso) {
    Node* novo_nodo = (Node*)malloc(sizeof(Node));
    novo_nodo->vertice = destino;
    novo_nodo->peso = peso;
    novo_nodo->proximo = lista[origem];
    lista[origem] = novo_nodo;

    // Para o grafo não direcionado, adicione a aresta na outra direção
    novo_nodo = (Node*)malloc(sizeof(Node));
    novo_nodo->vertice = origem;
    novo_nodo->peso = peso;
    novo_nodo->proximo = lista[destino];
    lista[destino] = novo_nodo;
}

// Função principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <n_nos> <nome_arquivo>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]); // Quantidade de nós
    char *nome_arquivo = argv[2]; // Nome do arquivo de saída

    // Verificar se a quantidade de nós é válida
    if (n <= 0) {
        printf("Número de nós deve ser maior que zero.\n");
        return 1;
    }

    // Abrir o arquivo para escrita
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo para escrita.\n");
        return 1;
    }

    // Inicializando a semente para números aleatórios
    srand(time(NULL));

    // Criando a matriz dinâmica
    int **matriz = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        matriz[i] = (int *)calloc(n, sizeof(int)); // Inicializar com 0
    }

    // Gerando uma árvore geradora
    for (int i = 1; i < n; i++) {
        int pai = rand() % i; // Escolhe um nó já conectado para ligar o próximo nó
        int peso = random_weight(1, MAX_PESO); // Garante pesos entre 1 e MAX_PESO
        matriz[i][pai] = peso;
        matriz[pai][i] = peso; // Grafo não direcionado
    }

    // Preenchendo as demais arestas aleatórias
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (matriz[i][j] == 0) { // Apenas onde não há aresta
                if (rand() % 2 == 0) { // 50% de probabilidade de criar uma aresta
                    int peso = random_weight(0, MAX_PESO);
                    matriz[i][j] = peso;
                    matriz[j][i] = peso; // Grafo não direcionado
                }
            }
        }
    }

    // Criando a lista de adjacência
    Node** lista_adjacencia = (Node**)malloc(n * sizeof(Node*));
    for (int i = 0; i < n; i++) {
        lista_adjacencia[i] = NULL; // Inicializa a lista como vazia
    }

    // Convertendo a matriz de adjacência para lista de adjacência
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matriz[i][j] != 0) { // Se existe uma aresta
                adicionar_aresta(lista_adjacencia, i, j, matriz[i][j]);
            }
        }
    }

// Escrevendo a lista de adjacência no arquivo
    fprintf(arquivo, "%d\n", n);
    for (int i = 0; i < n; i++) {
        fprintf(arquivo, "%d: ", i); // Imprime o índice do nó
        Node* atual = lista_adjacencia[i];
        while (atual != NULL) {
            fprintf(arquivo, "(%d, %d) ", atual->vertice, atual->peso); // Imprime o vértice e o peso
            atual = atual->proximo;
        }
        fprintf(arquivo, "\n");
    }

    // Liberando memória da lista de adjacência
    for (int i = 0; i < n; i++) {
        Node* atual = lista_adjacencia[i];
        while (atual != NULL) {
            Node* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
    }
    free(lista_adjacencia);

    // Liberando memória da matriz de adjacência
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
    fclose(arquivo);

    printf("Grafo gerado e salvo em '%s'.\n", nome_arquivo);

    return 0;
}