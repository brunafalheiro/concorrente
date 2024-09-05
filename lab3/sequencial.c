#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float *matrixA;
float *matrixB;
float *resultMatrix;
int rowsA, colsA, rowsB, colsB;

// Função para obter o tempo atual
#define GET_TIME(now) { \
    struct timespec time; \
    clock_gettime(CLOCK_REALTIME, &time); \
    now = time.tv_sec + time.tv_nsec / 1000000000.0; \
}

void multiply() {
    // Realiza a multiplicação das matrizes de forma sequencial
    for (size_t i = 0; i < rowsA; i++) {
        for (size_t j = 0; j < colsB; j++) {
            int res = 0;
            for (size_t k = 0; k < rowsA; k++) {
                res += matrixA[i * rowsA + k] * matrixB[k * rowsA + j];
            }
            resultMatrix[i * rowsA + j] = res;
        }
    }
}

int main(int argc, char *argv[])
{
    double startInit, startProcessing, startFinalization;
    double initDuration, processingDuration, finalizationDuration;

    long long int size;
    FILE *inputFileA = fopen(argv[1], "rb");
    FILE *inputFileB = fopen(argv[2], "rb");
    FILE *outputFile = fopen(argv[3], "wb");
    size_t ret;

    GET_TIME(startInit);
    initDuration = startInit;

    if (argc < 4) { 
        fprintf(stderr, "INPUT INVÁLIDO!\nUso: %s <Matriz 1> <Matriz 2> <Arquivo de Saída>\n", argv[0]);
        return 0;
    }

    if (!inputFileA) {
        fprintf(stderr, "Falha ao abrir o primeiro arquivo!\n");
        return 0;
    }

    if (!inputFileB) {
        fprintf(stderr, "Falha ao abrir o segundo arquivo!\n");
        return 0;
    }

    fread(&rowsA, sizeof(int), 1, inputFileA);
    fread(&colsA, sizeof(int), 1, inputFileA);

    size = colsA * rowsA;
    matrixA = (float *)malloc(sizeof(float) * size);
    if (!matrixA) {
        fprintf(stderr, "Erro de alocação de memória para a primeira matriz\n");
        return 0;
    }

    ret = fread(matrixA, sizeof(float), size, inputFileA);
    if (ret < size) {
        fprintf(stderr, "Erro ao ler os elementos da primeira matriz\n");
        return 0;
    }

    fclose(inputFileA);

    fread(&rowsB, sizeof(int), 1, inputFileB);
    fread(&colsB, sizeof(int), 1, inputFileB);

    size = colsB * rowsB;
    matrixB = (float *)malloc(sizeof(float) * size);
    if (!matrixB) {
        fprintf(stderr, "Erro de alocação de memória para a segunda matriz\n");
        return 0;
    }

    ret = fread(matrixB, sizeof(float), size, inputFileB);
    if (ret < size) {
        fprintf(stderr, "Erro ao ler os elementos da segunda matriz\n");
        return 0;
    }

    fclose(inputFileB);

    GET_TIME(startInit);
    initDuration = startInit - initDuration;

    GET_TIME(startProcessing);
    processingDuration = startProcessing;

    size = rowsA * colsB;
    resultMatrix = (float *)malloc(sizeof(float) * size);
    if (!resultMatrix) {
        fprintf(stderr, "Erro de alocação de memória para a matriz de resultado\n");
        return 0;
    }

    // Multiplicação das matrizes de forma sequencial
    multiply();

    GET_TIME(startProcessing);
    processingDuration = startProcessing - processingDuration;
    GET_TIME(startFinalization);
    finalizationDuration = startFinalization;

    // Escreve o número de linhas e colunas
    fwrite(&rowsA, sizeof(int), 1, outputFile);
    fwrite(&colsB, sizeof(int), 1, outputFile);

    // Escreve os elementos da matriz de resultado
    ret = fwrite(resultMatrix, sizeof(float), size, outputFile);
    if (ret < size) {
        fprintf(stderr, "Erro ao escrever no arquivo de saída\n");
        return 0;
    }

    fclose(outputFile);
    free(matrixA);
    free(matrixB);
    free(resultMatrix);

    GET_TIME(startFinalization);
    finalizationDuration = startFinalization - finalizationDuration;

    printf("FIM DA EXECUÇÃO:\nInicialização: %f ms\nProcessamento: %f ms\nFinalização: %f ms\n", initDuration, processingDuration, finalizationDuration);

    return 0;
}
