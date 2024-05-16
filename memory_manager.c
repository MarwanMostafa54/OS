#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 60
#define MAX_LINE_LENGTH 100

// Data structure for Process Control Block (PCB)
typedef struct {
    int process_id;
    char state[10];
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

MemoryWord memory[MEMORY_SIZE]; // Array to represent memory
int next_free_memory_index = 0; // Index to track the next free memory location

void initializeMemory() {
    // Initialize memory array to empty
    for (int i = 0; i < MEMORY_SIZE; i++) {
        strcpy(memory[i].name, "");
        strcpy(memory[i].data, "");
    }
}

void allocateMemory(int process_id, int lower_bound, int upper_bound) {
    PCB pcb;
    pcb.process_id = process_id;
    strcpy(pcb.state, "Ready");
    pcb.priority = 1; // Default priority
    pcb.program_counter = 0;
    pcb.lower_bound = lower_bound;
    pcb.upper_bound = upper_bound;

    // Store PCB in memory
    snprintf(memory[next_free_memory_index].data, sizeof(memory[next_free_memory_index].data), "%d %s %d %d %d %d",
             pcb.process_id, pcb.state, pcb.priority, pcb.program_counter, pcb.lower_bound, pcb.upper_bound);
    strcpy(memory[next_free_memory_index].name, "P"); // Prefix PCB name with 'P'
    next_free_memory_index++;

    printf("Allocated memory for Process %d\n", process_id);
}

void displayMemory() {
    printf("Memory Contents:\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d: %s - %s\n", i, memory[i].name, memory[i].data);
    }
}


int main() {
    initializeMemory();

    // Test memory allocation
    allocateMemory(1, 0, 2);
    allocateMemory(2, 3, 5);
    displayMemory();

    return 0;
}
