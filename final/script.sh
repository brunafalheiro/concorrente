#!/bin/bash

gcc -o gera_grafo gera_grafo.c
gcc -o concorrente concorrente.c -lpthread
gcc -o sequencial sequencial.c

NODES=500
PROBABILIDADE=95
NTHREADS=8

./gera_grafo $NODES matriz500
./sequencial matriz500 seq500
./concorrente matriz500 conc500 $NTHREADS

if diff conc500 seq500 > /dev/null; then
    echo "Teste de corretude completo. Sem falhas."
else
    echo "ERROR."
fi