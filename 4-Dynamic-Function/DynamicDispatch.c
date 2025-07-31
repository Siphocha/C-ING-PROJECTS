#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef int (*FileOperation)(const char* filename, void** result);

typedef struct {
    const char* name;
    FileOperation operation;
    const char* description;
} FileOperationInfo;

int addDataToFile(const char* filename, void** result);
int countLinesInFile(const char* filename, void** result);
int countCharsInFile(const char* filename, void** result);
int convertFileToUpper(const char* filename, void** result);
int convertFileToLower(const char* filename, void** result);
void Result(void* result);
void displayMenu(const FileOperationInfo* operations, int count);
int getOperationChoice(int max);
void* safeMalloc(size_t size);

const FileOperationInfo operations[] = {
    {"Append ", addDataToFile, "Add extra data"},
    {"Count lines", countLinesInFile, "Number lines in file"},
    {"Counts Chars", countCharsInFile, "Number of chars in file"},
    {"To Uppercase", convertFileToUpper, "Converts all chars to upper"},
    {"To Lowercase", convertFileToLower, "Converts all chars to lower"}
};

const int operationCount = sizeof(operations) / sizeof(operations[0]);

int main() {
    char filename[256];
    printf("filename to process: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Can't read file name");
        return 1;
    }

    //Remove newline character or characters just incase
    filename[strcspn(filename, "\n")] = '\0';

    while (1) {
        displayMenu(operations, operationCount);
        int choice = getOperationChoice(operationCount);

        if (choice == 0) {
            printf("Exiting...\n");
            break;
        }

        // Dynamically select and execute the function
        void* result = NULL;
        int status = operations[choice - 1].operation(filename, &result);

        if (status == 0) {
            if (result != NULL) {
                printf("Result: ");
                if (operations[choice - 1].operation == countLinesInFile ||
                    operations[choice - 1].operation == countCharsInFile) {
                    printf("%d\n", *(int*)result);
                } else if (operations[choice - 1].operation == addDataToFile) {
                    printf("%s\n", (char*)result);
                }
                Result(result);
            } else {
                printf("COMPLETE.\n");
            }
        } else {
            printf("FAILED\n");
            if (result != NULL) {
                Result(result);
            }
        }
    }

    return 0;
}

//Menu of the total list of options
void displayMenu(const FileOperationInfo* operations, int count) {
    printf("\nOperations List:\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s - %s\n", i + 1, operations[i].name, operations[i].description);
    }
    printf("0. Exit\n");
}

//Taking the users full constant choice
int getOperationChoice(int max) {
    int choice;
    printf("\nSelect an option (0-%d): ", max);

    while (1) {
        if (scanf("%d", &choice) != 1) {
            printf("Enter one of the numbers (0-%d): ", max);
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        if (choice >= 0 && choice <= max) {
            while (getchar() != '\n'); // Clear input buffer
            return choice;
        }

        printf("Enter one of the numbers %d: ", max);
    }
}

//Adding data to file when added new info
int addDataToFile(const char* filename, void** result) {
    char buffer[1024];
    printf("Data to add to the file: ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        perror("Error reading input");
        return -1;
    }

    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    if (fputs(buffer, file) == EOF) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }

    fclose(file);

    char* addedData = (char*)safeMalloc(strlen(buffer) + 1);
    strcpy(addedData, buffer);
    *result = addedData;

    return 0;
}

//Able to count lines in the file
int countLinesInFile(const char* filename, void** result) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int lines = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }

    if (lines > 0) {
        fseek(file, -1, SEEK_END);
        if (fgetc(file) != '\n') {
            lines++;
        }
    } else if (ftell(file) > 0) {
        lines = 1;
    }

    fclose(file);

    int* count = (int*)safeMalloc(sizeof(int));
    *count = lines;
    *result = count;

    return 0;
}

//Actually parsing the character types in the file
int countCharsInFile(const char* filename, void** result) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long chars = ftell(file);
    fclose(file);

    int* count = (int*)safeMalloc(sizeof(int));
    *count = (int)chars;
    *result = count;

    return 0;
}

//Chars to upper
int convertFileToUpper(const char* filename, void** result) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int ch;
    long pos = ftell(file);
    while ((ch = fgetc(file)) != EOF) {
        fseek(file, pos, SEEK_SET);
        fputc(toupper(ch), file);
        pos = ftell(file);
    }

    fclose(file);
    *result = NULL;
    return 0;
}

//function for chars to lower
int convertFileToLower(const char* filename, void** result) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int ch;
    long pos = ftell(file);
    while ((ch = fgetc(file)) != EOF) {
        fseek(file, pos, SEEK_SET);
        fputc(tolower(ch), file);
        pos = ftell(file);
    }

    fclose(file);
    *result = NULL;
    return 0;
}

//display all outputs of whichever outcome
void Result(void* result) {
    if (result != NULL) {
        free(result);
    }
}

//better be safe with memory
void* safeMalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}