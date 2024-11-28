#!/bin/bash

gcc -o gera_grafo gera_grafo.c
gcc -o concorrente concorrente.c -lpthread
gcc -o sequencial sequencial.c

# Parâmetros
NODES=500
PROBABILIDADE=95
NTHREADS_TEST1=4
NTHREADS_TEST2=8

MATRIZ_FILE="matriz${NODES}"
SEQ_FILE="seq${NODES}"
CONC_FILE="conc${NODES}"

# Executa os programas
./gera_grafo $NODES $PROBABILIDADE $MATRIZ_FILE
./sequencial $MATRIZ_FILE $SEQ_FILE
./concorrente $MATRIZ_FILE $CONC_FILE $NTHREADS_TEST1
./concorrente $MATRIZ_FILE $CONC_FILE $NTHREADS_TEST2

# Compara os resultados
if diff $SEQ_FILE $CONC_FILE > /dev/null; then
    echo ''
    echo "Teste de corretude completo. Sem falhas."
else
    echo ''
    echo "ERROR."
fi