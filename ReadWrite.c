#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 2

pthread_mutex_t mutex, writeblock;
int data = 0;
int reader_count = 0;

void* reader(void* arg) {
    int id = *((int*)arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        reader_count++;
        if (reader_count == 1) {
            pthread_mutex_lock(&writeblock);
        }
        pthread_mutex_unlock(&mutex);
        
        printf("Reader %d: read data = %d\n", id, data);
        sleep(1);
        
        pthread_mutex_lock(&mutex);
        reader_count--;
        if (reader_count == 0) {
            pthread_mutex_unlock(&writeblock);
        }
        pthread_mutex_unlock(&mutex);
        
        sleep(1);
    }
}

void* writer(void* arg) {
    int id = *((int*)arg);
    while (1) {
        pthread_mutex_lock(&writeblock);
        data++;
        printf("Writer %d: wrote data = %d\n", id, data);
        pthread_mutex_unlock(&writeblock);
        sleep(1);
    }
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&writeblock, NULL);
    
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&writeblock);
    
    return 0;
}
