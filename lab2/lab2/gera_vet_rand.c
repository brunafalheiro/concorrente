/*
Programa auxiliar para gerar um vetor de floats 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 //valor maximo de um elemento do vetor
//descomentar o define abaixo caso deseje imprimir uma versao do vetor gerado no formato texto
#define TEXTO 

int main(int argc, char*argv[]) {
   float *vetor1; //vetorque será gerada
   float *vetor2; //vetorque será gerada
   long int n; //qtde de elementos do vetor que serao gerados
   float elem; //valor gerado para incluir no vetor
   double soma1=0; //soma total dos elementos gerados
   double soma2=0; //soma total dos elementos gerados
   int fator=1; //fator multiplicador para gerar números negativos
   FILE * descritorArquivo; //descritor do arquivo de saida
   size_t ret; //retorno da funcao de escrita no arquivo de saida

   //recebe os argumentos de entrada
   if(argc < 3) {
      fprintf(stderr, "Digite: %s <dimensao> <arquivo saida>\n", argv[0]);
      return 1;
   }
   n = atoi(argv[1]);

   //aloca memoria para os vetores
   vetor1 = (float*) malloc(sizeof(float) * n);
   vetor2 = (float*) malloc(sizeof(float) * n);
   if(!vetor1 || !vetor2) {
      fprintf(stderr, "Erro de alocao da memoria do vetor\n");
      return 2;
   }

   //preenche o vetor com valores float aleatorios
   srand(time(NULL));
   for(long int i=0; i<n; i++) {
      elem = (rand() % MAX)/3.0 * fator;
      vetor1[i] = elem;
      soma1 += elem; //acumula o elemento na soma total

      elem = (rand() % MAX)/3.0 * fator;
      vetor2[i] = elem;
      soma2 += elem; //acumula o elemento na soma total
      fator*=-1;
   }

   //imprimir na saida padrao o vetor gerado
   #ifdef TEXTO
   fprintf(stdout, "%ld\n", n);
   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetor1[i]);
   }
   fprintf(stdout, "\n");

   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetor2[i]);
   }
   fprintf(stdout, "\n");
   #endif

   //escreve o vetor no arquivo
   //abre o arquivo para escrita binaria
   descritorArquivo = fopen(argv[2], "wb");
   if(!descritorArquivo) {
      fprintf(stderr, "Erro de abertura do arquivo\n");
      return 3;
   }
   //escreve a dimensao
   ret = fwrite(&n, sizeof(long int), 1, descritorArquivo);

   //escreve os elementos do primeiro vetor
   ret = fwrite(vetor1, sizeof(float), n, descritorArquivo);
   if(ret < n) {
      fprintf(stderr, "Erro de escrita no  arquivo\n");
      return 4;
   }

   //escreve os elementos do segundo vetor
   ret = fwrite(vetor2, sizeof(float), n, descritorArquivo);
   if(ret < n) {
      fprintf(stderr, "Erro de escrita no arquivo\n");
      return 4;
   }

   float produto_interno = 0;
   for (long int i = 0; i < n; i++) {
      produto_interno += vetor1[i] * vetor2[i];
   }

   printf("Produto interno: %f\n", produto_interno);

   //escreve o produto interno
   ret = fwrite(&produto_interno, sizeof(float), 1, descritorArquivo);
   if(ret < 1) {
      fprintf(stderr, "Erro de escrita no arquivo\n");
      return 4;
   }

   //finaliza o uso das variaveis
   fclose(descritorArquivo);
   free(vetor1);
   free(vetor2);
   return 0;
} 
