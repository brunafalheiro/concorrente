#include <stdio.h>
#include <stdlib.h>
#include "list_int.h"
#include <pthread.h>
#include "timer.h"

#define QTDE_OPS 10000000 //quantidade de operações sobre a lista
#define QTDE_INI 100 //quantidade de inserções iniciais na lista
#define MAX_VALUE 100 //valor máximo a ser inserido

//lista compartilhada iniciada 
struct list_node_s* head_p = NULL; 
//qtde de threads no programa
int nthreads;

// Variáveis de sincronização
pthread_mutex_t mutex;
pthread_cond_t cond_leitura, cond_escrita;
 // Contadores para leitura e escrita
int leitores = 0, escritores = 0, escritores_espera = 0;

void entraLeitura() {
    pthread_mutex_lock(&mutex);
    while (escritores > 0) { // Espera se houver escritor ativo
        pthread_cond_wait(&cond_leitura, &mutex);
    }
    leitores++; 
    pthread_mutex_unlock(&mutex);
}

void saiLeitura() {
    pthread_mutex_lock(&mutex);
    leitores--;
    if (leitores == 0 && escritores_espera > 0) {
        pthread_cond_signal(&cond_escrita); // Notifica escritores se não houver mais leitores
    }
    pthread_mutex_unlock(&mutex);
}

void entraEscrita() {
    pthread_mutex_lock(&mutex);
    escritores_espera++;
    while (leitores > 0 || escritores > 0) { // Espera até não haver leitores ou escritores ativos
        pthread_cond_wait(&cond_escrita, &mutex);
    }
    escritores_espera--; 
    escritores++; 
    pthread_mutex_unlock(&mutex);
}

void saiEscrita() {
    pthread_mutex_lock(&mutex);
    escritores--; 
    if (escritores_espera > 0) {
         // Prioriza a próxima operação de escrita
        pthread_cond_signal(&cond_escrita);
    } else {
        // Se não houver escritores esperando, libera leitores
        pthread_cond_broadcast(&cond_leitura);
    }
    pthread_mutex_unlock(&mutex);
}

// Tarefa das threads
void* tarefa(void* arg) {
    long int id = (long int) arg;
    int op;
    int in, out, read; 
    in = out = read = 0;

    // Realiza operações de consulta (98%), inserção (1%) e remoção (1%)
    for (long int i = id; i < QTDE_OPS; i += nthreads) {
        op = rand() % 100;
        if (op < 98) {
            entraLeitura();
            Member(i % MAX_VALUE, head_p); 
            saiLeitura();
            read++;
        } else if (98 <= op && op < 99) {
            entraEscrita();
            Insert(i % MAX_VALUE, &head_p);
            saiEscrita();
            in++;
        } else if (op >= 99) {
            entraEscrita();
            Delete(i % MAX_VALUE, &head_p); 
            saiEscrita();
            out++;
        }
    }
    // Registra a quantidade de operações realizadas por tipo
    printf("Thread %ld: in=%d out=%d read=%d\n", id, in, out, read);
    pthread_exit(NULL);
}

/* ---------------------------------------------------------------- */
int main(int argc, char* argv[]) {
    pthread_t* tid;
    double ini, fim, delta;

    //verifica se o número de threads foi passado na linha de comando
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    //insere os primeiros elementos na lista
    for (int i = 0; i < QTDE_INI; i++) {
        Insert(i % MAX_VALUE, &head_p);  /* Ignore return value */
    }

    //aloca espaco de memoria para o vetor de identificadores de threads no sistema
    tid = malloc(sizeof(pthread_t) * nthreads);
    if (tid == NULL) {
        printf("--ERRO: malloc()\n");
        return 2;
    }

    //tomada de tempo inicial
    GET_TIME(ini);

    //inicializa as variáveis de sincronização
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_leitura, NULL);
    pthread_cond_init(&cond_escrita, NULL);

    //cria as threads
    for (long int i = 0; i < nthreads; i++) {
        if (pthread_create(tid + i, NULL, tarefa, (void*) i)) {
            printf("--ERRO: pthread_create()\n");
            return 3;
        }
    }

    //aguarda as threads terminarem
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(*(tid + i), NULL)) {
            printf("--ERRO: pthread_join()\n");
            return 4;
        }
    }

    //tomada de tempo final
    GET_TIME(fim);
    delta = fim - ini;
    printf("Tempo: %lf\n", delta);

    // Libera as variáveis de sincronização
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_leitura);
    pthread_cond_destroy(&cond_escrita);

    //libera o espaço de memória do vetor de threads
    free(tid);

    //libera o espaço de memória da lista
    Free_list(&head_p);

    return 0;
}  /* main */