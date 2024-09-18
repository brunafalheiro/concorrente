#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond; //variavel condicional para sincronizacao
int impressos = 0; //contador de multiplos de 10 impressos
int pronto_para_imprimir = 0; //flag para controlar quando o valor esta pronto para ser impresso
int esperando = 0; //flag para controlar se a thread está esperando

//funcao executada pelas threads
void *ExecutaTarefa(void *arg) {
    long int id = (long int)arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        //--entrada na SC
        pthread_mutex_lock(&mutex);
        //--SC (seção critica)
        soma++; //incrementa a variavel compartilhada 

        // Se soma for múltiplo de 10, notifica a thread extra e aguarda
        if (soma % 10 == 0 && impressos < 20) {
            pronto_para_imprimir = 1;
            esperando = 1;
            pthread_cond_signal(&cond); // sinaliza para a thread extra
            // Aguarda até que o múltiplo de 10 seja impresso
            while (pronto_para_imprimir == 1) {
                pthread_cond_wait(&cond, &mutex);
            }
            esperando = 0;
        }

        //--saida da SC
        pthread_mutex_unlock(&mutex);
    }
    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra(void *args) {
    printf("Extra : esta executando...\n");
    while (impressos < 20) { // queremos imprimir 20 múltiplos de 10
        pthread_mutex_lock(&mutex);

        // espera até que um múltiplo de 10 esteja pronto para ser impresso
        while (pronto_para_imprimir == 0) {
            pthread_cond_wait(&cond, &mutex);
        }

        // Imprime o valor de soma
        if (soma % 10 == 0) {
            printf("soma = %ld \n", soma);
            impressos++;
        }

        // Libera a thread ExecutaTarefa para continuar
        pronto_para_imprimir = 0;
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }
    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema
    int nthreads; //qtde de threads (passada linha de comando)

    //--le e avalia os parametros de entrada
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    //--aloca as estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) {
        puts("ERRO--malloc");
        return 2;
    }

    //--inicilaiza o mutex e a variavel condicional
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    //--cria as threads
    if (pthread_create(&tid[0], NULL, ExecutaTarefa, (void*)t)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    //--cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    //--espera todas as threads terminarem
    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    //--finaliza o mutex e a variavel condicional
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Valor de 'soma' = %ld\n", soma);

    return 0;
}