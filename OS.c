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
#define MAX_PROCESSES 100

int next_pid = 1;
int pcb_count = 0;


// Define PCB structure
typedef struct {
    int pid;                // Process ID
    char state[20];         // Process State
    int priority;           // Current Priority
    int program_counter;    // Program Counter
    int lower_bound;        // Lower Bound of memory space
    int upper_bound;        // Upper Bound of memory space
} PCB;
PCB pcb_table[MAX_PROCESSES];
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

int get_next_pid() {
    return next_pid++;
}

// Function to allocate memory for a process
int allocate_memory(int pid, int num_lines_of_code, int *lower_bound, int *upper_bound) {
    printf("%d \n",pid);
    *lower_bound = -1;
    *upper_bound = -1;

    // Find consecutive empty slots in memory for the process
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strcmp(memory[i].name, "") == 0) {
            if (*lower_bound == -1) {
                *lower_bound = i;
            }
            if (i - *lower_bound + 1 >= num_lines_of_code) {
                *upper_bound = i;
                break;
            }
        } else {
            *lower_bound = -1;  // Reset lower_bound if non-empty slot encountered
        }
    }

    // Mark allocated memory slots with process ID
    if (*lower_bound != -1 && *upper_bound != -1) {
        for (int i = *lower_bound; i <= *upper_bound; i++) {
            sprintf(memory[i].name, "Process%d", pid);
        }
        return 0;  // Success
    }

    return -1;  // Failure
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
    
    if(strstr(resource_name,"serInput")!= NULL){
        resource_name="userInput"; 
    }else if(strstr(resource_name,"serOutput")!= NULL){
        resource_name="userOutput"; 
    }
    printf("%s \n",resource_name);
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
    if(strstr(resource_name,"serInput")!= NULL){
        resource_name="userInput"; 
    }else if(strstr(resource_name,"serOutput")!= NULL){
        resource_name="userOutput"; 
    }
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
 printf(" %s\n", instruction);

     char *Input = strtok(instruction, " ");
    if (Input == NULL) {
        printf("Invalid instruction: %s\n", instruction);
        return;
    }
printf("Token1: %s \n",Input);
    // Check the instruction type
    if (strcmp(Input, "print") == 0) {
        // Print the value
        Input = strtok(NULL, " ");
        if (Input != NULL) {
            printf("%s\n", Input);
        } else {
            printf("Invalid print instruction\n");
        }
    } else if (strcmp(Input, "assign") == 0) {
       
        char variable[10];
        char value[20];
        Input = strtok(NULL, " ");
        printf("Token2: %s \n",Input);
        if (Input != NULL) {
            strcpy(variable, Input);
            int num;
                    scanf("%d", &num);
                    strcpy(value, Input);
                    printf("Assigned value %d to variable %s\n", num, variable);
            } else {
                printf("Invalid assign instruction\n");
            }
    } else if (strcmp(Input, "writeFile") == 0) {
    // Write data to a file
    Input = strtok(NULL, " ");
    if (Input != NULL) {
      char filename[20];
      strcpy(filename, Input);
      Input = strtok(NULL, " ");
      if (Input != NULL) {
        // Implement logic to write data (Input) to the file (filename)
        // You can open the file in append mode ("a") and write the data
        FILE *file = fopen(filename, "a");
        if (file != NULL) {
          fprintf(file, "%s\n", Input);
          fclose(file);
          printf("Data written to file %s\n", filename);
        } else {
          printf("Error opening file for writing: %s\n", filename);
        }
      } else {
        printf("Invalid writeFile instruction\n");
      }
    } else {
      printf("Invalid writeFile instruction\n");
    }
  } else if (strcmp(Input, "readFile") == 0) {
    // Read data from a file
    Input = strtok(NULL, " ");
    if (Input != NULL) {
      char filename[20];
      strcpy(filename, Input);
      // Implement logic to read data from the file (filename)
      // You can open the file in read mode ("r") and read the first line
      FILE *file = fopen(filename, "r");
      if (file != NULL) {
        char data[50];
        if (fgets(data, sizeof(data), file) != NULL) {
          // Remove newline character if present
          char *newline = strchr(data, '\n');
          if (newline != NULL) {
            *newline = '\0';
          }
          printf("Data read from file %s: %s\n", filename, data);
        } else {
          printf("File is empty or error reading\n");
        }
        fclose(file);
      } else {
        printf("Error opening file for reading: %s\n", filename);
      }
    } else {
      printf("Invalid readFile instruction\n");
    }
  } else if (strcmp(Input, "printFromTo") == 0) {
    Input = strtok(NULL, " ");
    
    if (Input != NULL) {
      int start, end;
      char value_a[20], value_b[20];
      if (sscanf(Input, "%s", value_a) == 1) { // Read value into variable_a
        Input = strtok(NULL, " ");
        
        if (Input != NULL) {
          if (sscanf(Input, "%s", value_b) == 1) { // Read value into variable_b
            sscanf(value_a, "%d", &start);
            sscanf(value_b, "%d", &end);
            
            for (int i = start; i <= end; i++) {
              printf("%d ", i);
            }
            
          } else {
            printf("Invalid printFromTo instruction (invalid end value)\n");
          }
        } else {
          printf("Invalid printFromTo instruction (missing end value)\n");
        }
      } else {
        printf("Invalid printFromTo instruction (invalid start value)\n");
      }
    } else {
      printf("Invalid printFromTo instruction (missing start value)\n");
    }
  } else if (strcmp(Input, "semWait") == 0) {
    // Acquire a resource

    
    if (Input != NULL) {
    Input = strtok(NULL, " ");
      char* resource_name = strdup(Input); 
        printf("Token: %s\n", resource_name);
        semWait(resource_name, 1); 
    } else {
      printf("Invalid semWait instruction\n");
    }
  } else if (strcmp(Input, "semSignal") == 0) {
    // Release a resource
    Input = strtok(NULL, " ");
    if (Input != NULL) { 
      semSignal(Input);
    } else {
      printf("Invalid semSignal instruction\n");
    }
  } else {
    printf("Unknown instruction: %s\n", Input);
  }
}

// Function to read and interpret instructions from a text file
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    int pid = get_next_pid();
    char line[MAX_LINE_LENGTH];
    int line_count = 0;

    // First pass: count the number of lines
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
    }
    rewind(file); // Reset file pointer to the beginning

    // Allocate memory for the lines of code
    int lower_bound, upper_bound;
    if (allocate_memory(pid, line_count, &lower_bound, &upper_bound) == -1) {
        printf("Error: Not enough memory to allocate for process %d\n", pid);
        fclose(file);
        return;
    }

    // Create PCB for the process
    PCB pcb;
    pcb.pid = pid;
    strcpy(pcb.state, "Not Ready");
    pcb.priority = 0;  // Default priority
    pcb.program_counter = lower_bound;
    pcb.lower_bound = lower_bound;
    pcb.upper_bound = upper_bound;

    // Store the PCB in the PCB table
    pcb_table[pcb_count++] = pcb;

    // Second pass: read the lines and store them in memory
    int current_address = lower_bound;
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Store the line in the memory array
        strcpy(memory[current_address].data, line);
        current_address++;
    }

    fclose(file);
}



// int main() {
//   const char *filename = "Program_1.txt";
//     interpret_file(filename);
//     return 0;

// }

int main() {
    initialize_memory();
    interpret_file("Program_1.txt");

    // interpret_file("Program_2.txt");

    // interpret_file("Program_3.txt");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strcmp(memory[i].name, "") != 0) {
            printf("Memory[%d] Name: %s, Data: %s\n", i, memory[i].name, memory[i].data);
        }
    }

    for (int i = 0; i < pcb_count; i++) {
        PCB pcb = pcb_table[i];
        printf("PCB[%d] PID: %d, State: %s, Priority: %d, PC: %d, Lower: %d, Upper: %d\n",
            i, pcb.pid, pcb.state, pcb.priority, pcb.program_counter, pcb.lower_bound, pcb.upper_bound);
    }

    // Process currPro = select_next_process();
    
    // Fetch the instruction from the memory using the program counter of the current process
 
for (int i = 0; i < 7; i++) {
    // Execute the fetched instruction
    char *currInst = memory[i].data;
    printf("%s",memory[i].data);
    execute_instruction(currInst);
}
    return 0;
}

