#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PESO 20 // Peso máximo para as arestas

// Função para gerar um número aleatório dentro de um intervalo
int random_weight(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Função principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <quantidade_de_nos> <nome_do_arquivo>\n", argv[0]);
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

    // Gerando uma árvore geradora mínima (MST)
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

    // Escrevendo no arquivo
    fprintf(arquivo, "%d\n", n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(arquivo, "%d ", matriz[i][j]);
        }
        fprintf(arquivo, "\n");
    }

    // Liberando memória e fechando arquivo
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
    fclose(arquivo);

    printf("Grafo gerado e salvo em '%s'.\n", nome_arquivo);

    return 0;
}