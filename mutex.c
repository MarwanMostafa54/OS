#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_MUTEXES 3

// Enum to represent process states
typedef enum {
    READY,
    BLOCKED
} ProcessState;

// Data structure for Process Control Block (PCB)
typedef struct {
    int process_id;
    ProcessState state;
    int priority;
    int program_counter;
    int lower_bound;
    int upper_bound;
} PCB;

// Data structure to represent a mutex
typedef struct {
    int id;
    int available;
    int blocked_processes[MAX_PROCESSES];
    int num_blocked_processes;
} Mutex;

Mutex mutexes[MAX_MUTEXES]; // Array to store mutexes
PCB processes[MAX_PROCESSES]; // Array to store processes
int num_processes = 0;

// Function to initialize mutexes
void initializeMutexes() {
    for (int i = 0; i < MAX_MUTEXES; i++) {
        mutexes[i].id = i;
        mutexes[i].available = 1; // Mutex initially available
        mutexes[i].num_blocked_processes = 0;
    }
}

// Function to perform semWait operation on a mutex
void semWait(int mutex_id, PCB *process) {
    if (mutexes[mutex_id].available) {
        mutexes[mutex_id].available = 0; // Mutex is now unavailable
    } else {
        // Block the process
        process->state = BLOCKED;
        mutexes[mutex_id].blocked_processes[mutexes[mutex_id].num_blocked_processes++] = process->process_id;
    }
}

// Function to perform semSignal operation on a mutex
void semSignal(int mutex_id) {
    mutexes[mutex_id].available = 1; // Mutex is now available

    // Unblock a process waiting on this mutex, if any
    if (mutexes[mutex_id].num_blocked_processes > 0) {
        int process_id = mutexes[mutex_id].blocked_processes[--mutexes[mutex_id].num_blocked_processes];
        processes[process_id].state = READY;
    }
}

int main() {
    // Initialize mutexes
    initializeMutexes();

    // Test semWait and semSignal operations
    PCB process1 = {0, READY, 1, 0, 0, 2};
    semWait(0, &process1);
    printf("Process 1 blocked on mutex 0\n");

    PCB process2 = {1, READY, 1, 0, 3, 5};
    semWait(0, &process2); // This process will be blocked
    printf("Process 2 blocked on mutex 0\n");

    semSignal(0); // Unblock Process 1
    printf("Process 1 unblocked\n");

    semSignal(0); // Unblock Process 2
    printf("Process 2 unblocked\n");

    return 0;
}
