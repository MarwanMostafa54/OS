#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

void executeProgram(const char *filename);

int main() {
    const char *filename = "program_1.txt";
    executeProgram(filename);
    return 0;
}

void print(int x) {
    printf("%d\n", x);
}

void assign(int *x, int y) {
    *x = y;
}

void writeFile(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s\n", data);
        fclose(file);
    } else {
        printf("Error: Cannot open file %s\n", filename);
    }
}

void readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
            printf("%s", buffer);
        }
        fclose(file);
    } else {
        printf("Error: Cannot open file %s\n", filename);
    }
}

void printFromTo(int x, int y) {
    for (int i = x; i <= y; i++) {
        printf("%d ", i);
    }
    printf("\n");
}

void executeInstruction(const char *instruction) {
    char *token = strtok((char *)instruction, " ");
    if (strcmp(token, "print") == 0) {
        int x = atoi(strtok(NULL, " "));
        print(x);
    } else if (strcmp(token, "assign") == 0) {
        int *x;
        x = malloc(sizeof(int));
        char *variable = strtok(NULL, " ");
        int y = atoi(strtok(NULL, " "));
        assign(x, y);
        printf("Variable %s assigned value %d\n", variable, *x);
        free(x);
    } else if (strcmp(token, "writeFile") == 0) {
        char *filename = strtok(NULL, " ");
        char *data = strtok(NULL, " ");
        writeFile(filename, data);
    } else if (strcmp(token, "readFile") == 0) {
        char *filename = strtok(NULL, " ");
        readFile(filename);
    } else if (strcmp(token, "printFromTo") == 0) {
        int x = atoi(strtok(NULL, " "));
        int y = atoi(strtok(NULL, " "));
        printFromTo(x, y);
    } else {
        printf("Error: Invalid instruction\n");
    }
}

void executeProgram(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
            executeInstruction(buffer);
        }
        fclose(file);
    } else {
        printf("Error: Cannot open file %s\n", filename);
    }
}
