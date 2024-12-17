#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Tipurile de fir
#define WHITE 0
#define BLACK 1

// Variabile globale pentru sincronizare
pthread_mutex_t resource_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t white_queue, black_queue;

int active_white = 0;   // numar de fire albe active
int active_black = 0;   // numar de fire negre active
int waiting_white = 0;  // fire albe in asteptare
int waiting_black = 0;  // fire negre in asteptare
int turn = WHITE;       // Politica de alternanta

void *thread_function(void *arg) {
    int type = *(int *)arg;

    if (type == WHITE) {
        pthread_mutex_lock(&resource_mutex);
        waiting_white++;
        while (active_black > 0 || (turn == BLACK && waiting_black > 0)) {
            pthread_mutex_unlock(&resource_mutex);
            sem_wait(&white_queue);
            pthread_mutex_lock(&resource_mutex);
        }
        waiting_white--;
        active_white++;
        turn = BLACK;  // Schimba prioritatea catre BLACK
        pthread_mutex_unlock(&resource_mutex);

        // Utilizeaza resursa
        printf("White thread using resource.\n");
        sleep(1);

        pthread_mutex_lock(&resource_mutex);
        active_white--;
        if (active_white == 0) {
            // Elibereaza firele negre daca exista in asteptare
            while (waiting_black > 0) {
                sem_post(&black_queue);
                waiting_black--;
            }
        }
        pthread_mutex_unlock(&resource_mutex);

    } else if (type == BLACK) {
        pthread_mutex_lock(&resource_mutex);
        waiting_black++;
        while (active_white > 0 || (turn == WHITE && waiting_white > 0)) {
            pthread_mutex_unlock(&resource_mutex);
            sem_wait(&black_queue);
            pthread_mutex_lock(&resource_mutex);
        }
        waiting_black--;
        active_black++;
        turn = WHITE;  // Schimba prioritatea catre WHITE
        pthread_mutex_unlock(&resource_mutex);

        // Utilizeaza resursa
        printf("Black thread using resource.\n");
        sleep(1);

        pthread_mutex_lock(&resource_mutex);
        active_black--;
        if (active_black == 0) {
            // Elibereaza firele albe daca exista in asteptare
            while (waiting_white > 0) {
                sem_post(&white_queue);
                waiting_white--;
            }
        }
        pthread_mutex_unlock(&resource_mutex);
    }

    return NULL;
}

int main() {
    pthread_t threads[20];
    int thread_types[20];

    // Genereaza fire aleatoriu
    for (int i = 0; i < 20; i++) {
        thread_types[i] = rand() % 2;
    }

    // Initializeaza semafoarele
    sem_init(&white_queue, 0, 0);
    sem_init(&black_queue, 0, 0);

    for (int i = 0; i < 20; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_types[i]);
    }

    for (int i = 0; i < 20; i++) {
        pthread_join(threads[i], NULL);
    }

    // Distruge semafoarele
    sem_destroy(&white_queue);
    sem_destroy(&black_queue);

    return 0;
}
