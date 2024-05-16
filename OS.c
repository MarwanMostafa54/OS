#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 60
#define MAX_LINE_LENGTH 100
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

// Data structure to represent a memory word
typedef struct {
    char name[20];
    char data[MAX_LINE_LENGTH];
} MemoryWord;

// Data structure to represent a mutex
typedef struct {
    int id;
    int available;
} Mutex;

MemoryWord memory[MEMORY_SIZE]; // Array to represent memory
Mutex mutexes[MAX_MUTEXES]; // Array to store mutexes
PCB processes[MAX_PROCESSES]; // Array to store processes
int num_processes = 0;
int next_free_memory_index = 0; // Index to track the next free memory location

// Function to initialize memory
void initializeMemory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        strcpy(memory[i].name, "");
        strcpy(memory[i].data, "");
    }
}

// Function to initialize mutexes
void initializeMutexes() {
    for (int i = 0; i < MAX_MUTEXES; i++) {
        mutexes[i].id = i;
        mutexes[i].available = 1; // Mutex initially available
    }
}

// Function to allocate memory for a process
void allocateMemory(int process_id, int lower_bound, int upper_bound) {
    PCB pcb;
    pcb.process_id = process_id;
    pcb.state = READY;
    pcb.priority = 1; // Default priority
    pcb.program_counter = 0;
    pcb.lower_bound = lower_bound;
    pcb.upper_bound = upper_bound;

    // Store PCB in memory
    snprintf(memory[next_free_memory_index].data, sizeof(memory[next_free_memory_index].data), "%d %d %d %d %d %d",
             pcb.process_id, pcb.state, pcb.priority, pcb.program_counter, pcb.lower_bound, pcb.upper_bound);
    strcpy(memory[next_free_memory_index].name, "P"); // Prefix PCB name with 'P'
    next_free_memory_index++;

    printf("Allocated memory for Process %d\n", process_id);
}

// Function to perform semWait operation on a mutex
void semWait(int mutex_id, PCB *process) {
    if (mutexes[mutex_id].available) {
        mutexes[mutex_id].available = 0; // Mutex is now unavailable
    } else {
        // Block the process
        process->state = BLOCKED;
        printf("Process %d blocked on mutex %d\n", process->process_id, mutex_id);
    }
}

// Function to perform semSignal operation on a mutex
void semSignal(int mutex_id) {
    mutexes[mutex_id].available = 1; // Mutex is now available
    printf("Mutex %d signaled\n", mutex_id);
}

// Function to execute an instruction
void executeInstruction(PCB *process, const char *instruction) {
    char *token = strtok((char *)instruction, " ");
    if (strcmp(token, "print") == 0) {
        int x = atoi(strtok(NULL, " "));
        printf("Process %d: Printing %d\n", process->process_id, x);
    } else if (strcmp(token, "assign") == 0) {
        char *variable = strtok(NULL, " ");
        int y = atoi(strtok(NULL, " "));
        printf("Process %d: Assigning %d to %s\n", process->process_id, y, variable);
    } else if (strcmp(token, "writeFile") == 0) {
        char *filename = strtok(NULL, " ");
        char *data = strtok(NULL, " ");
        printf("Process %d: Writing data to %s\n", process->process_id, filename);
    } else if (strcmp(token, "readFile") == 0) {
        char *filename = strtok(NULL, " ");
        printf("Process %d: Reading data from %s\n", process->process_id, filename);
    } else if (strcmp(token, "printFromTo") == 0) {
        int x = atoi(strtok(NULL, " "));
        int y = atoi(strtok(NULL, " "));
        printf("Process %d: Printing numbers from %d to %d\n", process->process_id, x, y);
    } else if (strcmp(token, "semWait") == 0) {
        int mutex_id = atoi(strtok(NULL, " "));
        semWait(mutex_id, process);
    } else if (strcmp(token, "semSignal") == 0) {
        int mutex_id = atoi(strtok(NULL, " "));
        semSignal(mutex_id);
    } else {
        printf("Error: Invalid instruction\n");
    }
}

// Function to execute a program
void executeProgram(PCB *process, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
            executeInstruction(process, buffer);
        }
        fclose(file);
    } else {
        printf("Error: Cannot open file %s\n", filename);
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
                executeProgram(process, "program.txt");
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

// Function to display memory contents
void displayMemory() {
    printf("Memory Contents:\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d: %s - %s\n", i, memory[i].name, memory[i].data);
    }
}

int main() {
    initializeMemory(); // Initialize memory
    initializeMutexes(); // Initialize mutexes

    // Example: Create some processes
    allocateMemory(0, 0, 2);
    allocateMemory(1, 3, 5);
    num_processes = 2;

    // Schedule processes
    scheduleProcesses();

    // Display memory contents
    displayMemory();

    return 0;
}
