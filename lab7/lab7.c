#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define N 10
#define BUFFER_SIZE N

char buffer1[BUFFER_SIZE];
char buffer2[BUFFER_SIZE];
// semaforos para indicar que os buffers estao cheios
sem_t sem_fill_buffer1, sem_fill_buffer2;
// semaforos para inficar que os buffers estao vazios
sem_t sem_empty_buffer1, sem_empty_buffer2;

void* thread1(void* arg) {
// lê o arquivo e preenche buffer1
    FILE* file = fopen("entrada.txt", "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(0);
    }
    while (fgets(buffer1, BUFFER_SIZE, file)) {
        // espera espaco no buffer 1 e informa que esta preenchido
        sem_wait(&sem_empty_buffer1);
        sem_post(&sem_fill_buffer1);
    }
    fclose(file);
    pthread_exit(NULL);
}

void* thread2(void* arg) {
    int line_size = 1;
    int count = 0;
    int maxLeitura = 0;
    int n = 0;

    while (1) {
        // espera que tenha dados no buffer1 e espaco no buffer2
        sem_wait(&sem_fill_buffer1);
        sem_wait(&sem_empty_buffer2);

        int len = strlen(buffer1);
        int j = 0;

        for (int i = 0; i < len; i++) {
            buffer2[j++] = buffer1[i];
            count++;

            if (count == line_size) {
                buffer2[j++] = '\n';
                count = 0;

                // aumenta ou diminui a quantidade de caracteres a serem impressos
                if (!maxLeitura) {
                    n++;
                    line_size = 2*n+1;
                    if (line_size > 19) {  // 19 pq é o último valor válido (2n + 1)
                        maxLeitura = 1;
                        line_size = 10; // limita a imprimir só 10 caracteres por linha
                    }
                }
            }
        }

        buffer2[j] = '\0';
        sem_post(&sem_fill_buffer2);  // buffer2 está preenchido
        sem_post(&sem_empty_buffer1); // libera buffer1
    }
    pthread_exit(NULL);
}

void* thread3(void* arg) {
    while (1) {
        sem_wait(&sem_fill_buffer2);  // espera que buffer2 esteja preenchido
        printf("%s", buffer2);
        sem_post(&sem_empty_buffer2);
    }
    pthread_exit(NULL);
}

int main() {
    sem_init(&sem_fill_buffer1, 0, 0);
    sem_init(&sem_fill_buffer2, 0, 0);
    sem_init(&sem_empty_buffer1, 0, 1);
    sem_init(&sem_empty_buffer2, 0, 1);

    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_create(&t3, NULL, thread3, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&sem_fill_buffer1);
    sem_destroy(&sem_fill_buffer2);
    sem_destroy(&sem_empty_buffer1);
    sem_destroy(&sem_empty_buffer2);

    return 0;
}
