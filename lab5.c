#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define SHARED_MEM_NAME "/shared_mem_modified"
#define SEMAPHORE_NAME "/semaphore_modified"
#define MEM_SIZE sizeof(int)

int coin_flip() {
    return rand() % 2;
}

int main() {
    int shm_fd, *shared_value;
    sem_t *semaphore;

    srand((unsigned int)(time(NULL) ^ getpid()));

    // Creare sau deschidere segment de memorie partajată
    shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        perror("Error opening shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, MEM_SIZE) == -1) {
        perror("Error resizing shared memory");
        exit(EXIT_FAILURE);
    }

    shared_value = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_value == MAP_FAILED) {
        perror("Error mapping shared memory");
        exit(EXIT_FAILURE);
    }

    // Creare sau deschidere semafor
    semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("Error opening semaphore");
        exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(semaphore);

        int value = *shared_value;

        if (value >= 1000) {
            sem_post(semaphore);
            break;
        }

        printf("Process %d read value: %d\n", getpid(), value);

        while (coin_flip() == 1) {
            (*shared_value)++;
            printf("Process %d incremented value to: %d\n", getpid(), *shared_value);
        }

        sem_post(semaphore);
        usleep(100000);
    }

    munmap(shared_value, MEM_SIZE);
    close(shm_fd);
    sem_close(semaphore);

    if (getpid() % 2 == 0) {
        shm_unlink(SHARED_MEM_NAME);
        sem_unlink(SEMAPHORE_NAME);
    }

    return 0;
}
