package main

import (
    "fmt"
    "math"
)

func ehPrimo(n int) bool {
    if n <= 1 { return false }
    if n == 2 { return true }
    if n % 2 == 0 { return false }
    for i := 3; i <= int(math.Sqrt(float64(n))); i += 2 {
        if n % i == 0 { return false }
    }
    return true
}

func main() {
    N := 100 
    routines := 10

    nums := make(chan int, N) // canal de números
    results := make(chan bool, N) // canal de resultados
    count := make(chan int) // canal de contagem

    for i := 0; i < routines; i++ {
        go func() {
            for num := range nums { results <- ehPrimo(num) }
            count <- 1
        }()
    }

    // Envia os números para o canal nums
    go func() {
        for i := 1; i <= N; i++ { nums <- i }
        close(nums)
    }()

    // Fecha o canal quando todas as goroutines terminarem
    go func() {
        for i := 0; i < routines; i++ { <-count }
        close(results)
    }()

    countPrimes := 0
    for isPrime := range results {
        if isPrime { countPrimes++ }
    }

    fmt.Printf("Total de números primos: %d\n", countPrimes)
}