/* Disciplina: Programacao Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório: 2 */
/* Codigo: usando threads em C com passagem e retorno de um argumento */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

//--funcao executada pelas threads
void *PrintHello (void* arg) {
   //typecasting do argumento
   long int idThread = (long int) arg; //typecasting do argumento. precisa converter de volta para utilizar como inteiro
   long int retorno = idThread*2;
   //log da thread
   printf("Thread: %ld retorno=%ld\n", idThread, retorno);

   pthread_exit((void*) retorno);
}

//--funcao principal do programa
int main(int argc, char * argv[]) {
   int nthreads; //qtde de threads que serao criadas (recebida na linha de comando)
   long int retorno;

   //verifica se o argumento 'qtde de threads' foi passado e armazena seu valor
   if(argc<2) {
       printf("--ERRO: informe a qtde de threads <%s> <nthreads>\n", argv[0]);
       return 1;
   }
   nthreads = atoi(argv[1]);

   //identificadores das threads no sistema
   pthread_t tid_sistema[nthreads];

   //cria as threads e atribui a tarefa de cada thread
   for(long int i=1; i<=nthreads; i++) {
      printf("--Cria a thread %ld\n", i);
      if (pthread_create(&tid_sistema[i-1], NULL, PrintHello, (void*) i)) { // Converte o inteiro i para um ponteiro genérico (void *)
         printf("--ERRO: pthread_create()\n"); 
	 return 2;
      }
   }
   
   //espera todas as threads terminarem e recebe os valores retornados
   for (int i=0; i<nthreads; i++) {
     if (pthread_join(tid_sistema[i], (void**) &retorno)) {
         printf("--ERRO: pthread_join() da thread %d\n", i);
     }
     printf("Thread %d retornou %ld \n", i+1, retorno);
   }

   //log da função main
   printf("--Thread principal terminou\n");
   return 0;
}







