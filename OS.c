#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 60
#define MAX_PROCESS_VARS 3


#define NUM_PRIORITY_LEVELS 4
#define BASE_QUANTUM 1


#define FILE_RESOURCE "file"
#define USER_INPUT_RESOURCE "userInput"
#define USER_OUTPUT_RESOURCE "userOutput"

#define MAX_LINE_LENGTH 100

// Define PCB structure
typedef struct {
    int pid;                // Process ID
    char state[20];         // Process State
    int priority;           // Current Priority
    int program_counter;    // Program Counter
    int lower_bound;        // Lower Bound of memory space
    int upper_bound;        // Upper Bound of memory space
} PCB;

typedef struct {
    char name[20];
    char data[50];
} MemoryWord;

typedef struct {
    int pid;
    int priority;
} Process;


typedef struct {
    char name[20];          // Name of the resource
    int locked;             // Indicates if the resource is locked (1) or not (0)
    int blocked_queue[100]; // Queue to store PIDs of blocked processes
    int num_blocked;        // Number of processes blocked on this resource
} Mutex;

// Define a structure to represent the main memory
MemoryWord memory[MEMORY_SIZE];

Mutex mutexes[3] = {
    {FILE_RESOURCE, 0, {0}, 0},
    {USER_INPUT_RESOURCE, 0, {0}, 0},
    {USER_OUTPUT_RESOURCE, 0, {0}, 0}
};
// Function to initialize memory
void initialize_memory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        strcpy(memory[i].name, "");
        strcpy(memory[i].data, "");
    }
}

// Function to allocate memory for a process
int allocate_memory(int pid, int num_lines_of_code) {
    int lower_bound = -1;
    int upper_bound = -1;

    // Find consecutive empty slots in memory for the process
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strcmp(memory[i].name, "") == 0) {
            if (lower_bound == -1) {
                lower_bound = i;
            }
            if (i - lower_bound + 1 >= num_lines_of_code) {
                upper_bound = i;
                break;
            }
        } else {
            lower_bound = -1;  // Reset lower_bound if non-empty slot encountered
        }
    }

    // Mark allocated memory slots with process ID
    if (lower_bound != -1 && upper_bound != -1) {
        for (int i = lower_bound; i <= upper_bound; i++) {
            sprintf(memory[i].name, "Process%d", pid);
        }
    }

    return lower_bound;  // Return lower bound of allocated memory space
}
// Define array of ready queues for different priority levels
Process ready_queues[NUM_PRIORITY_LEVELS][100]; // Assuming a maximum of 100 processes per queue
int num_processes[NUM_PRIORITY_LEVELS] = {0}; // Number of processes in each ready queue

// Define array to store quantum for each priority level
int quantum[NUM_PRIORITY_LEVELS] = {BASE_QUANTUM, BASE_QUANTUM * 2, BASE_QUANTUM * 4, BASE_QUANTUM * 8};

// Function to add process to ready queue
void add_to_ready_queue(Process process) {
    ready_queues[process.priority - 1][num_processes[process.priority - 1]] = process;
    num_processes[process.priority - 1]++;
}

// Function to select next process for execution
Process select_next_process() {
    Process next_process;

    // Iterate through priority levels starting from highest
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        // Check if there are processes in the current priority level
        if (num_processes[i] > 0) {
            // Select the first process in the ready queue of the current priority level
            next_process = ready_queues[i][0];

            // Move processes in the ready queue
            for (int j = 0; j < num_processes[i] - 1; j++) {
                ready_queues[i][j] = ready_queues[i][j + 1];
            }

            // Decrement number of processes in the ready queue
            num_processes[i]--;
            break;
        }
    }

    return next_process;
} 

void semWait(char resource_name[], int pid) {
    // Find the mutex corresponding to the resource
    int index = -1;
    for (int i = 0; i < 3; i++) {
        if (strcmp(mutexes[i].name, resource_name) == 0) {
            index = i;
            break;
        }
    }

    // If the resource is not found, return
    if (index == -1) {
        printf("Resource not found\n");
        return;
    }

    // If the resource is locked, block the process and add it to the blocked queue
    if (mutexes[index].locked) {
        mutexes[index].blocked_queue[mutexes[index].num_blocked] = pid;
        mutexes[index].num_blocked++;
        printf("Process %d blocked on resource %s\n", pid, resource_name);
    } else {
        mutexes[index].locked = 1; // Lock the resource
        printf("Process %d acquired resource %s\n", pid, resource_name);
    }
}
void semSignal(char resource_name[]) {
    // Find the mutex corresponding to the resource
    int index = -1;
    for (int i = 0; i < 3; i++) {
        if (strcmp(mutexes[i].name, resource_name) == 0) {
            index = i;
            break;
        }
    }

    // If the resource is not found, return
    if (index == -1) {
        printf("Resource not found\n");
        return;
    }

    // If there are blocked processes, unblock the highest priority process
    if (mutexes[index].num_blocked > 0) {
        int pid_to_unblock = mutexes[index].blocked_queue[0]; // Assuming highest priority is at index 0
        // Move other blocked processes forward in the array
        for (int i = 1; i < mutexes[index].num_blocked; i++) {
            mutexes[index].blocked_queue[i - 1] = mutexes[index].blocked_queue[i];
        }
        mutexes[index].num_blocked--;

        // Unblock the process (you can implement logic to move the unblocked process to its ready queue based on priority)
        printf("Process %d unblocked on resource %s\n", pid_to_unblock, resource_name);
    } else {
        // Otherwise, unlock the resource
        mutexes[index].locked = 0;
        printf("Resource %s released\n", resource_name);
    }
}

void execute_instruction(char *instruction) {
    char *token = strtok(instruction, " ");
    if (token == NULL) {
        printf("Invalid instruction: %s\n", instruction);
        return;
    }

    // Check the instruction type
    if (strcmp(token, "print") == 0) {
        // Print the value
        token = strtok(NULL, " ");
        if (token != NULL) {
            printf("%s\n", token);
        } else {
            printf("Invalid print instruction\n");
        }
    } else if (strcmp(token, "assign") == 0) {
        // Assign a value
        char variable[10];
        char value[20];
        token = strtok(NULL, " ");
        if (token != NULL) {
            strcpy(variable, token);
            token = strtok(NULL, " ");
            if (token != NULL) {
                if (strcmp(token, "input") == 0) {
                    printf("Please enter a value for variable %s: ", variable);
                    scanf("%s", value);
                    printf("Assigned value %s to variable %s\n", value, variable);
                } else {
                    strcpy(value, token);
                    printf("Assigned value %s to variable %s\n", value, variable);
                }
            } else {
                printf("Invalid assign instruction\n");
            }
        } else {
            printf("Invalid assign instruction\n");
        }
    } else if (strcmp(token, "writeFile") == 0) {
        // Write data to a file
        // Implementation left as an exercise
    } else if (strcmp(token, "readFile") == 0) {
        // Read data from a file
        // Implementation left as an exercise
    } else if (strcmp(token, "printFromTo") == 0) {
        // Print numbers from x to y
        // Implementation left as an exercise
    } else if (strcmp(token, "semWait") == 0) {
        // Acquire a resource
        token = strtok(NULL, " ");
        if (token != NULL) {
            printf("Waiting for resource: %s\n", token);
            // Implement semaphore wait logic
        } else {
            printf("Invalid semWait instruction\n");
        }
    } else if (strcmp(token, "semSignal") == 0) {
        // Release a resource
        token = strtok(NULL, " ");
        if (token != NULL) {
            printf("Releasing resource: %s\n", token);
            // Implement semaphore signal logic
        } else {
            printf("Invalid semSignal instruction\n");
        }
    } else {
        printf("Unknown instruction: %s\n", token);
    }
}

// Function to read and interpret instructions from a text file
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Execute instruction
        execute_instruction(line);
    }

    fclose(file);
}

int main() {
  const char *filename = "Program_3.txt";
    interpret_file(filename);
    return 0;
}
