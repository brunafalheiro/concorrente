#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_WEIGHT 20 // Peso máximo para as arestas

// Função para gerar um número aleatório dentro de um intervalo
int random_weight() {
    return rand() % MAX_WEIGHT + 1;
}

// Função para adicionar arestas garantindo que o nó inicial tenha caminhos para todos os outros
void add_edges(FILE *arquivo, int n) {
    // Garantindo que cada nó tenha pelo menos uma aresta conectada ao nó anterior
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                fprintf(arquivo, "0 "); // Diagonal 0 (sem laço)
            } else if (j == 0 && i != 0) {
                // Conectando todos os nós ao nó 0, exceto o próprio nó 0
                fprintf(arquivo, "%d ", random_weight());
            } else if (rand() % 2 == 0) {
                // Adiciona um peso aleatório nas arestas com 50% de probabilidade
                fprintf(arquivo, "%d ", random_weight());
            } else {
                fprintf(arquivo, "0 "); // Não há aresta
            }
        }
        fprintf(arquivo, "\n");
    }
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

    // Escrevendo o número de nós no arquivo
    fprintf(arquivo, "%d\n", n);

    // Gerando a matriz de adjacência (grafo)
    // Adiciona arestas que garantem que todos os nós sejam acessíveis a partir do nó 0
    add_edges(arquivo, n);

    // Fechando o arquivo
    fclose(arquivo);

    printf("Grafo gerado e salvo em '%s'.\n", nome_arquivo);

    return 0;
}
