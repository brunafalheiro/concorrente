#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *vector;
int N, M;

// Function to increment each element of the vector by 1
void *increment(void *arg) {
  int id = *(int *)arg;
  for (int i = id; i < N; i += M) {
    vector[i]++;
  }
  return NULL;
}

// Function to verify the result
int* verify_result() {
  for (int i = 0; i < N; i++) {
    if (vector[i] != 1) {
      printf("Error at index %d: %d\n", i, vector[i]);
      return 0;
    }
  }
  printf("All values are correct!\n");
  return vector;
}

int main(int argc, char *argv[]) {
  // Check if the correct number of arguments is provided
  if (argc != 3) {
    printf("Usage: %s\nM: Number of threads\nN: Number of integer elements\n", argv[0]);
    return 1;
  }

  M = atoi(argv[1]);
  N = atoi(argv[2]);

  pthread_t threads[M];
  int thread_ids[M];

  // Initialize the vector
  vector = (int *)malloc(N * sizeof(int));
  for (int i = 0; i < N; i++) {
    vector[i] = 0;
  }

  // Create threads
  for (int i = 0; i < M; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, increment, (void*)&thread_ids[i]);
  }

  // Wait for all threads to finish
  for (int i = 0; i < M; i++) {
    pthread_join(threads[i], NULL);
  }

  // Verify the result
  int *result_vector = verify_result();

  // Print the vector
  printf("Vector: [ ");
  for (int i = 0; i < N; i++) {
    printf("%d ", result_vector[i]);
  }
  printf("]\n");

  // Free the allocated memory
  free(vector);
  return 0;
}
