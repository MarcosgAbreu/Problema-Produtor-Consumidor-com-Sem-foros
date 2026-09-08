/*
  Produtor-Consumidor com semaforos (POSIX)
  Uso: ./produtor-consumidor <N> <Np> <Nc>

  N  = tamanho da memoria compartilhada (vetor)
  Np = numero de threads produtoras
  Nc = numero de threads consumidoras
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define M 10000
#define MAX_ALEATORIO 10000000

int N;
int *memoria;
int produzidos;
int consumidos;
int terminado;

sem_t mutex;  
sem_t vazios; 
sem_t cheios; 

int achar_livre(void) {
    int i;
    for (i = 0; i < N; i++) {
        if (memoria[i] == 0) {
            return i;
        }
    }
    return -1;
}

int achar_ocupada(void) {
    int i;
    for (i = 0; i < N; i++) {
        if (memoria[i] != 0) {
            return i;
        }
    }
    return -1;
}

int eh_primo(int n) {
    int i;

    if (n < 2) {
        return 0;
    }

    for (i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }

    return 1;
}

void acordar_todos(void) {
    int i;

    for (i = 0; i < N; i++) {
        sem_post(&vazios);
        sem_post(&cheios);
    }
}

void *produtor(void *arg) {
    int pos, valor;

    (void)arg;

    while (1) {
        
        sem_wait(&vazios);

        sem_wait(&mutex);

        
        if (terminado || produzidos >= M) {
            sem_post(&mutex);
            sem_post(&vazios);
            break;
        }

        pos = achar_livre();
        valor = (rand() % MAX_ALEATORIO) + 1;
        memoria[pos] = valor;
        produzidos++;

        sem_post(&mutex);
        
        sem_post(&cheios);
    }

    return NULL;
}

void *consumidor(void *arg) {
    int pos, local, fim;

    (void)arg;

    while (1) {
        
        sem_wait(&cheios);

        sem_wait(&mutex);

        if (terminado) {
            sem_post(&mutex);
            sem_post(&cheios);
            break;
        }

        pos = achar_ocupada();

        
        local = memoria[pos];
        memoria[pos] = 0;
        consumidos++;

        fim = (consumidos >= M);
        if (fim) {
            terminado = 1;
        }

        sem_post(&mutex);
        
        sem_post(&vazios);

        
        if (eh_primo(local)) {
            printf("%d eh primo\n", local);
        } else {
            printf("%d nao eh primo\n", local);
        }

        if (fim) {
            acordar_todos();
            break;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int Np, Nc;
    int i;
    pthread_t *t_prod;
    pthread_t *t_cons;
    struct timespec inicio, fim;
    double tempo;

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <N> <Np> <Nc>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    Np = atoi(argv[2]);
    Nc = atoi(argv[3]);

    if (N <= 0 || Np <= 0 || Nc <= 0) {
        fprintf(stderr, "N, Np e Nc devem ser positivos.\n");
        return 1;
    }

    memoria = calloc(N, sizeof(int));
    t_prod = malloc(sizeof(pthread_t) * Np);
    t_cons = malloc(sizeof(pthread_t) * Nc);

    produzidos = 0;
    consumidos = 0;
    terminado = 0;

    sem_init(&mutex, 0, 1);
    sem_init(&vazios, 0, N);
    sem_init(&cheios, 0, 0);

    srand(time(NULL));

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (i = 0; i < Np; i++) {
        pthread_create(&t_prod[i], NULL, produtor, NULL);
    }

    for (i = 0; i < Nc; i++) {
        pthread_create(&t_cons[i], NULL, consumidor, NULL);
    }

    for (i = 0; i < Np; i++) {
        pthread_join(t_prod[i], NULL);
    }

    for (i = 0; i < Nc; i++) {
        pthread_join(t_cons[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = (fim.tv_sec - inicio.tv_sec) +
            (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    
    fprintf(stderr, "N=%d Np=%d Nc=%d consumidos=%d tempo=%.6f\n",
            N, Np, Nc, consumidos, tempo);

    sem_destroy(&mutex);
    sem_destroy(&vazios);
    sem_destroy(&cheios);

    free(memoria);
    free(t_prod);
    free(t_cons);

    return 0;
}
