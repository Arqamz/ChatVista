// PRODUCER (WRITE)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_KEY 2345
#define SHM_SIZE 1024

int main() {
    int shmid;
    void *shared_memory;
    char buff[100];
    sem_t sem_full, sem_empty;

    // Create shared memory
    shmid = shmget((key_t)SHM_KEY, SHM_SIZE, 0666|IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    printf("Key of the shared memory is: %d\n", shmid);

    // Attach shared memory
    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    printf("Process attached at: %p\n", shared_memory);

    // Initialize semaphores
    sem_init(&sem_full, 1, 0);   // 1 for shared between processes, initial value 0
    sem_init(&sem_empty, 1, 1);  // 1 for shared between processes, initial value 1

    while (1) {
        printf("WRITE SOMETHING INTO SHARED MEMORY: ");
        fgets(buff, sizeof(buff), stdin);

        sem_wait(&sem_empty);  // Wait if no empty slots
        strcpy((char *)shared_memory, buff);
        sem_post(&sem_full);   // Signal that data is available

        printf("Data written to shared memory: %s\n", (char *)shared_memory);
    }

    // Clean up semaphores
    sem_destroy(&sem_full);
    sem_destroy(&sem_empty);
    return 0;
}


// CONSUMER (READ)


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_KEY 2345
#define SHM_SIZE 1024

int main() {
    int shmid;
    void *shared_memory;
    sem_t sem_full, sem_empty;

    // Get shared memory
    shmid = shmget((key_t)SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    printf("Key of the shared memory is: %d\n", shmid);

    // Attach shared memory
    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    printf("Process attached at: %p\n", shared_memory);

    // Initialize semaphores
    sem_init(&sem_full, 1, 0);   // 1 for shared between processes, initial value 0
    sem_init(&sem_empty, 1, 1);  // 1 for shared between processes, initial value 1

    while (1) {
        sem_wait(&sem_full);  // Wait if no data
        printf("Data read from shared memory: %s\n", (char *)shared_memory);
        sem_post(&sem_empty); // Signal that empty slot is available

        sleep(1);  // Simulate processing delay
    }

    // Clean up semaphores
    sem_destroy(&sem_full);
    sem_destroy(&sem_empty);
    return 0;
}
