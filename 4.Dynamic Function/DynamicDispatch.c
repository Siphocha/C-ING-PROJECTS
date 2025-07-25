#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//File operations so must declare pointer types preemptively
typedef int (*FileOperation)(const char* filename, void** result);

//Classic struct to hold operation info
typedef struct {
    const char* name;
    FileOperation operation;
    const char* description;
} FileOperationInfo;

//Function declarations from planning stage of necessary functions
int addDataToFile(const char* filename, void** result);
int countLinesInFile(const char* filename, void** result);
int countCharsInFile(const char* filename, void** result);
int convertFileToUpper(const char* filename, void** result);
int convertFileToLower(const char* filename, void** result);
void freeResult(void* result);
void displayMenu(const FileOperationInfo* operations, int count);
int getOperationChoice(int max);
void* safeMalloc(size_t size);

//Available file operations
const FileOperationInfo operations[] = {
    {"Adding Data", addDataToFile, "Data to file is appended"},
    {"Countng Lines", countLinesInFile, "Number of lines in file are counted"},
};

const int operationCount = sizeof(operations) / sizeof(operations[0]);

int main() {
    char filename[256];
    printf("Enter the filename to process: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Error reading file name");
        return 1;
    }
}