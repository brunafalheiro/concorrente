#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

float *matrixA;
float *matrixB;
float *resultMatrix;
int rowsA, colsA, rowsB, colsB;
int nThreads;

// Function to get the current time (time.h was throwing a warning)
#define GET_TIME(now) { \
    struct timespec time; \
    clock_gettime(CLOCK_REALTIME, &time); \
    now = time.tv_sec + time.tv_nsec / 1000000000.0; \
}

void *multiply(void *arg) {
    long int id = (long int)arg;


    // Each thread processes a subset of rows from matrixA
    for (size_t i = id; i < rowsA; i += nThreads) {
        for (size_t j = 0; j < colsB; j++) {
            int res = 0;
            for (size_t k = 0; k < rowsA; k++) {
                res += matrixA[i * rowsA + k] * matrixB[k * rowsA + j];
            }
            resultMatrix[i * rowsA + j] = res;
        }
    }
    return NULL;
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

    if (argc < 5) { 
        fprintf(stderr, "INVALID INPUT!\nUsage: %s <Matrix 1> <Matrix 2> <Output File> <N Threads>\n", argv[0]);
        return 0;
    }

    if (!inputFileA) {
        fprintf(stderr, "Failed to open the first file!\n");
        return 0;
    }

    if (!inputFileB) {
        fprintf(stderr, "Failed to open the second file!\n");
        return 0;
    }

    fread(&rowsA, sizeof(int), 1, inputFileA);
    fread(&colsA, sizeof(int), 1, inputFileA);

    size = colsA * rowsA;
    matrixA = (float *)malloc(sizeof(float) * size);
    if (!matrixA) {
        fprintf(stderr, "Memory allocation error for the first matrix\n");
        return 0;
    }

    ret = fread(matrixA, sizeof(float), size, inputFileA);
    if (ret < size) {
        fprintf(stderr, "Error reading the first matrix elements\n");
        return 0;
    }

    fclose(inputFileA);

    fread(&rowsB, sizeof(int), 1, inputFileB);
    fread(&colsB, sizeof(int), 1, inputFileB);

    size = colsB * rowsB;
    matrixB = (float *)malloc(sizeof(float) * size);
    if (!matrixB) {
        fprintf(stderr, "Memory allocation error for the second matrix\n");
        return 0;
    }

    ret = fread(matrixB, sizeof(float), size, inputFileB);
    if (ret < size) {
        fprintf(stderr, "Error reading the second matrix elements\n");
        return 0;
    }

    fclose(inputFileB);

    GET_TIME(startInit);
    initDuration = startInit - initDuration;

    GET_TIME(startProcessing);
    processingDuration = startProcessing;

    nThreads = atoi(argv[4]);
    pthread_t threads[nThreads];
    long int threadIds[nThreads];

    size = rowsA * colsB;
    resultMatrix = (float *)malloc(sizeof(float) * size);

    if (!resultMatrix) {
        fprintf(stderr, "Memory allocation error for the result matrix\n");
        return 0;
    }

    // Create threads for matrix multiplication
    for (int i = 0; i < nThreads; i++) {
        threadIds[i] = i;
        if (pthread_create(&threads[i], NULL, multiply, (void *)threadIds[i])) {
            fprintf(stderr, "ERROR--pthread_create\n");
            return 0;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < nThreads; i++) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "ERROR: pthread_join() \n");
            return 0;
        }
    }

    GET_TIME(startProcessing);
    processingDuration = startProcessing - processingDuration;
    GET_TIME(startFinalization);
    finalizationDuration = startFinalization;

    // Write the number of rows and columns
    fwrite(&rowsA, sizeof(int), 1, outputFile);
    fwrite(&colsB, sizeof(int), 1, outputFile);

    // Write the result matrix elements
    ret = fwrite(resultMatrix, sizeof(float), size, outputFile);
    if (ret < size) {
        fprintf(stderr, "Error writing to the output file\n");
        return 0;
    }

    fclose(outputFile);
    free(matrixA);
    free(matrixB);
    free(resultMatrix);

    GET_TIME(startFinalization);
    finalizationDuration = startFinalization - finalizationDuration;

    printf("END OF EXECUTION:\nInitialization: %f ms\nProcessing: %f ms\nFinalization: %f ms\n", initDuration, processingDuration, finalizationDuration);

    return 0;
}
