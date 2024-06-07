#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

int choosing[NUM_THREADS];
int tickets[NUM_THREADS];

void enter_critical_section(int thread_id) {
    choosing[thread_id] = 1;
    __sync_synchronize();
    
    int max_ticket = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (tickets[i] > max_ticket) {
            max_ticket = tickets[i];
        }
    }
    tickets[thread_id] = max_ticket + 1;
    __sync_synchronize();
    
    choosing[thread_id] = 0;
    __sync_synchronize();
    
    for (int i = 0; i < NUM_THREADS; i++) {
        while (choosing[i]) {
            // Busy-wait
        }
        
        while (tickets[i] != 0 && (tickets[i] < tickets[thread_id] || 
              (tickets[i] == tickets[thread_id] && i < thread_id))) {
            // Busy-wait
        }
    }
}

void exit_critical_section(int thread_id) {
    tickets[thread_id] = 0;
}

void* thread_function(void* arg) {
    int thread_id = *((int*)arg);
    
    while (1) {
        enter_critical_section(thread_id);
        
        // Critical section
        printf("Thread %d is in the critical section.\n", thread_id);
        sleep(1);
        
        exit_critical_section(thread_id);
        
        // Non-critical section
        printf("Thread %d is in the non-critical section.\n", thread_id);
        sleep(1);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        choosing[i] = 0;
        tickets[i] = 0;
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}
