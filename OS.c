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
int a_lower, a_upper, b_lower, b_upper;
int pid_a = 4; 
int pid_b = 5; 
int num_lines_of_code = 1; 
char value_a[50];
char value_b[50];


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
    int blocked;
} Process;

Process processes[3];

typedef struct {
    char name[20];          // Name of the resource
    int locked;             // Indicates if the resource is locked (1) or not (0)
    int blocked_queue[100]; // Queue to store PIDs of blocked processes
    int num_blocked;        // Number of processes blocked on this resource
} Mutex;
int a;
int b;
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
            if(pid>3){
            sprintf(memory[i].name, "Stored Data %d", pid);
            }else{  
            sprintf(memory[i].name, "Process %d", pid);
         }        
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
    int priority = process.priority - 1;
    ready_queues[priority][num_processes[priority]] = process;
    num_processes[priority]++;
}

// Function to select next process for execution
Process select_next_process() {
    Process next_process = {0, 0, 0};
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        if (num_processes[i] > 0) {
            next_process = ready_queues[i][0];
            for (int j = 0; j < num_processes[i] - 1; j++) {
                ready_queues[i][j] = ready_queues[i][j + 1];
            }
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
    }else if(strstr(resource_name,"ile")!= NULL){
        resource_name="file"; 
    }
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
        processes[pid - 1].blocked = 1;
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
    }else if(strstr(resource_name,"ile")!= NULL){
        resource_name="file"; 
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
        processes[pid_to_unblock - 1].blocked = 0;
        // Unblock the process (you can implement logic to move the unblocked process to its ready queue based on priority)
        printf("Process %d unblocked on resource %s\n", pid_to_unblock, resource_name);
    } else {
        // Otherwise, unlock the resource
        mutexes[index].locked = 0;
        printf("Resource %s released\n", resource_name);
    }
}

void execute_instruction(PCB *pcb) {
  if (pcb->program_counter > pcb->upper_bound) {
        strcpy(pcb->state, "Terminated");
        return;
    }

    printf("\n\nExecuting instruction: %s\n", memory[pcb->program_counter].data);

    char *instruction = strdup(memory[pcb->program_counter].data);

     char *Input = strtok(instruction, " ");
    if (Input == NULL) {
        printf("Invalid instruction: %s\n", instruction);
        free(instruction);
        return;
    }
    // Check the instruction type
    if (strcmp(Input, "print") == 0) {
        // Print the value
        Input = strtok(NULL, " ");
        if (Input != NULL) {
          printf("%s \n", memory[b_lower].data); 
        } else {
            printf("Invalid print instruction\n");
        }
    } else if (strcmp(Input, "assign") == 0) {                
        Input = strtok(NULL, " ");
        
        if (Input != NULL) {
                    // printf("%s",Input);
                    if(strcmp(Input, "a") == 0){
                     Input = strtok(NULL, " ");
                     if(strcmp(Input,"readFile")==0){
                    Input = strtok(NULL, " ");
                if (Input != NULL) {
                FILE *file = fopen(memory[b_lower].data, "r");
                if (file != NULL) {
                    char data[50];
                    if (fgets(data, sizeof(data), file) != NULL) {
                    char *newline = strchr(data, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }
                    printf("Data read from file %s: %s\n", memory[b_lower].data, data);
                    sprintf(memory[a_lower].data, "%s", data);
                    } else {
                    printf("File is empty or error reading\n");
                    }
                    fclose(file);
                } else {
                    printf("Error opening file for reading: %s\n", memory[b_lower].data);
                }
                }
                  }
            else{
                       if (allocate_memory(pid_a, num_lines_of_code, &a_lower, &a_upper) == 0) {
                        pid_a+=2;
                            scanf("%s", value_a);
                        sprintf(memory[a_lower].data, "%s", value_a);
                            printf("Memory allocated for variable 'a'\n");
                            } else {
                            printf("Failed to allocate memory for variable 'a'\n");
                            }
                  }
                  }else if(strcmp(Input, "b") == 0){
                        Input = strtok(NULL, " ");
                if(strcmp(Input,"readFile")==0){
                    Input = strtok(NULL, " ");
                if (Input != NULL) {
                FILE *file = fopen(memory[a_lower].data, "r");
                if (file != NULL) {
                    char data[50];
                    if (fgets(data, sizeof(data), file) != NULL) {
                    char *newline = strchr(data, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }
                    printf("Data read from file %s: %s\n", memory[a_lower].data, data);
                    sprintf(memory[b_lower].data, "%s", data);
                    } else {
                    printf("File is empty or error reading\n");
                    }
                    fclose(file);
                } else {
                    printf("Error opening file for reading: %s\n", memory[a_lower].data);
                }
                }
                  
            }else{
                       if (allocate_memory(pid_b, num_lines_of_code, &b_lower, &b_upper) == 0) {
                        pid_b+=2;
                                scanf("%s", value_b);
                        sprintf(memory[b_lower].data, "%s", value_b);

                            printf("Memory allocated for variable 'b'\n");
                        } else {
                            printf("Failed to allocate memory for variable 'b'\n");
                        }                    
                    }
            }
             else {
                printf("Invalid assign instruction\n");
            }
        }
    } else if (strcmp(Input, "writeFile") == 0) {
    Input = strtok(NULL, " ");
    if (Input != NULL) {
    FILE *file = fopen(memory[a_lower].data, "w"); // Open file for writing
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }
    fprintf(file, "%s", memory[b_lower].data); // Write data to file
    fclose(file); // Close file
    printf("Data written to %s successfully.\n", memory[a_lower].data);
    } else {
      printf("Invalid writeFile instruction\n");
    }
  } else if (strcmp(Input, "printFromTo") == 0) {
   
    if (Input != NULL) {
            for (int i = atoi(memory[a_lower].data); i <= atoi(memory[b_lower].data); i++) {
              printf("%d \n", i);
            }
      }
  } else if (strcmp(Input, "semWait") == 0) {
    // Acquire a resource

    
    if (Input != NULL) {
    Input = strtok(NULL, " ");
    if(Input != NULL){
      char* resource_name = strdup(Input); 
        semWait(resource_name, pcb->pid); 
        free(resource_name);
    }
    } else {
      printf("Invalid semWait instruction\n");
    }
  } else if (strcmp(Input, "semSignal") == 0) {
    // Release a resource
    Input = strtok(NULL, " ");
    if (Input != NULL) { 
      char* resource_name = strdup(Input);
      semSignal(resource_name);
      free(resource_name);
    } else {
      printf("Invalid semSignal instruction\n");
    }
  } else {
    printf("Unknown instruction: %s\n", Input);
  }

  free(instruction);
  pcb->program_counter++;
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

    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
    }
    rewind(file);

    int lower_bound, upper_bound;
    if (allocate_memory(pid, line_count, &lower_bound, &upper_bound) == -1) {
        printf("Error: Not enough memory to allocate for process %d\n", pid);
        fclose(file);
        return;
    }
    pcb_table[pcb_count++] = (PCB) {pid, "READY", 1, lower_bound, lower_bound, upper_bound};

    int line_number = lower_bound;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        strcpy(memory[line_number].data, line);
        line_number++;
    }
    fclose(file);
    Process p = {pid, 1, 0};
    processes[pid - 1] = p;
    add_to_ready_queue(p);
    
}

int main() {
    initialize_memory();
    // Interpret and load programs into memory
    interpret_file("Program_1.txt");
    interpret_file("Program_2.txt");
    interpret_file("Program_3.txt");
for (int i = 0; i < 3; i++) {
    strcpy(memory[23 + (i * 6)].name, "pcbID");
    sprintf(memory[23 + (i * 6)].data, "%d", pcb_table[i].pid); // Convert int to string

    strcpy(memory[24 + (i * 6)].name, "Priority");
    sprintf(memory[24 + (i * 6)].data, "%d", pcb_table[i].priority); // Convert int to string

    strcpy(memory[25 + (i * 6)].name, "State");
    strcpy(memory[25 + (i * 6)].data, pcb_table[i].state); // Copy state string directly

    strcpy(memory[26 + (i * 6)].name, "program_counter");
    sprintf(memory[26 + (i * 6)].data, "%d", pcb_table[i].program_counter); // Convert int to string

    strcpy(memory[27 + (i * 6)].name, "Lower Bound");
    sprintf(memory[27 + (i * 6)].data, "%d", pcb_table[i].lower_bound); // Convert int to string

    strcpy(memory[28 + (i * 6)].name, "Upper Bound");
    sprintf(memory[28 + (i * 6)].data, "%d", pcb_table[i].upper_bound); // Convert int to string
}

    // Display memory contents and PCB information for debugging
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strcmp(memory[i].name, "") != 0) {
            printf("Memory[%d] Name: %s, Data: %s\n", i, memory[i].name, memory[i].data);
        }
    }

    for (int i = 0; i < pcb_count; i++) {
        PCB pcb = pcb_table[i];
        printf("PCB[%d] PID: %d, State: %s, Priority: %d, PC: %d, Lower: %d, Upper: %d\n",
            i, pcb.pid, pcb.state, pcb.priority, pcb.program_counter, pcb.lower_bound, pcb.upper_bound);
        strcpy(pcb.state, "Ready"); // Set initial state to "Ready"
    }

    int current_quantum_1 = 1;
    int current_quantum_2 = 1;
    int current_quantum_3 = 1;

    while (1) {
        int process_executed = 0;
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            while (num_processes[i] > 0) {
                Process currPro = select_next_process();
                PCB *pcb = &pcb_table[currPro.pid - 1];
                strcpy(pcb->state, "Running");
                printf("\nRunning Process: PID %d\n", pcb->pid);

                // Print ready processes
                printf("Ready Processes: ");
                for (int j = 0; j < NUM_PRIORITY_LEVELS; j++) {
                    for (int k = 0; k < num_processes[j]; k++) {
                        printf("PID %d (Priority %d), ", ready_queues[j][k].pid, ready_queues[j][k].priority);
                    }
                }
                printf("\n");

                if(processes[pcb->pid  - 1].blocked == 0){
                    if(pcb->pid == 1){
                        for(int i = 0; i < current_quantum_1; i++){
                            execute_instruction(pcb);
                        }
                        current_quantum_1 *=2 ;
                    } 
                    if(pcb->pid == 2){
                        for(int i = 0; i < current_quantum_2; i++){
                            execute_instruction(pcb);
                        }
                        current_quantum_2 *=2 ;
                    } 
                    if(pcb->pid == 3){
                        for(int i = 0; i < current_quantum_3; i++){
                            execute_instruction(pcb);
                        }
                        current_quantum_3 *=2 ;
                    }
                    printf("\n"); 
                    for (int i = 0; i < MEMORY_SIZE; i++) {
                        if (strcmp(memory[i].name, "") != 0) {
                            printf("Memory[%d] Name: %s, Data: %s\n", i, memory[i].name, memory[i].data);
                        }
                    }
                }

                if (strcmp(pcb->state, "Terminated") == 0) {
                    printf("\nProcess %d terminated\n", pcb->pid);
                } else {
                    strcpy(pcb->state, "Ready");
                    add_to_ready_queue(currPro); // Re-add the process to the ready queue if not terminated
                }

                process_executed = 1;
                break; // Move to the next priority level after executing one process
            }

            if (process_executed) {
                break;
            }
        }

        if (!process_executed) {
            break; // Exit the loop if no process was executed (all processes terminated or blocked)
        }
    }

    return 0;
}

