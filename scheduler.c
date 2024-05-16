#include <stdio.h>
#include <stdlib.h>

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
} Mutex;

Mutex mutexes[MAX_MUTEXES]; // Array to store mutexes
PCB processes[MAX_PROCESSES]; // Array to store processes
int num_processes = 0;

// Function to initialize mutexes
void initializeMutexes() {
    for (int i = 0; i < MAX_MUTEXES; i++) {
        mutexes[i].id = i;
        mutexes[i].available = 1; // Mutex initially available
    }
}

// Function to schedule processes using multilevel feedback model
void scheduleProcesses() {
    // Iterate through priority levels (1 to 4)
    for (int priority = 1; priority <= 4; priority++) {
        // Iterate through processes
        for (int i = 0; i < num_processes; i++) {
            PCB *process = &processes[i];
            // Check if process is in ready state and has the current priority
            if (process->state == READY && process->priority == priority) {
                // Execute process
                printf("Executing Process %d\n", process->process_id);
                // Increment program counter
                process->program_counter++;
                // Check if process has completed its execution
                if (process->program_counter >= process->upper_bound) {
                    // Reset program counter
                    process->program_counter = 0;
                    // Lower process priority (if not already at lowest priority)
                    if (process->priority < 4) {
                        process->priority++;
                    }
                }
            }
        }
    }
}

int main() {
    // Initialize mutexes
    initializeMutexes();

    // Example: Create some processes
    processes[num_processes++] = (PCB){0, READY, 1, 0, 0, 2};
    processes[num_processes++] = (PCB){1, READY, 2, 0, 3, 5};
    processes[num_processes++] = (PCB){2, READY, 3, 0, 6, 8};

    // Schedule processes
    scheduleProcesses();

    return 0;
}
