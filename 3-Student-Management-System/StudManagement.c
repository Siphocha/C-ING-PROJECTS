#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 100
#define MAX_NAME_LENGTH 50
#define MAX_COURSE_LENGTH 30
#define FILENAME "students.txt"

typedef struct {
    char name[MAX_NAME_LENGTH];
    int id;
    int age;
    char subjects[MAX_COURSE_LENGTH];
    float grades[15];
    float average;
} Student;

Student students[MAX_STUDENTS];
int studentsCount = 0;

void loadStudents();
void saveStudents();
void addStudent();
void displayStudents();
void searchStudent();
void updateStudent();
void deleteStudent();
void sortStudents();
void calculateAverage(Student *student);
void displayMenu();
void clearInputBuffer();

int main() {
    loadStudents();

    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch(choice) {
            case 1:
                addStudent();
                break;
            case 2:
                displayStudents();
                break;
            case 3:
                searchStudent();
                break;
            case 4:
                updateStudent();
                break;
            case 5:
                deleteStudent();
                break;
            case 6:
                sortStudents();
                break;
            case 7:
                saveStudents();
                printf("Will put something here\n");
                break;
            default:
                printf("Will put something here\n");
        }
    } while(choice != 7);

    return 0;
}

void loadStudents() {
    FILE *file = fopen(FILENAME, "rb");
    if (file == NULL) {
        return;
    }
    studentsCount = fread(students, sizeof(Student), MAX_STUDENTS, file);
    fclose(file);
}

void saveStudents() {
    FILE *file = fopen(FILENAME, "wb");
    if (file == NULL) {
        printf("Cant save data, whats wrong?\n");
        return;
    }
    fwrite(students, sizeof(Student), studentsCount, file);
    fclose(file);
}

void addStudent() {
    if (studentsCount >= MAX_STUDENTS) {
        printf("Maximum number of students, no more!\n");
        return;
    }

    Student newStudent;

    printf("Enter student name: ");
    fgets(newStudent.name, MAX_NAME_LENGTH, stdin);
    newStudent.name[strcspn(newStudent.name, "\n")] = '\0';

    printf("Enter student ID: ");
    scanf("%d", &newStudent.id);
    clearInputBuffer();

    for (int i = 0; i < studentsCount; i++) {
        if (students[i].id == newStudent.id) {
            printf("ID already exists, try again!\n");
            return;
        }
    }

    printf("Enter student age: ");
    scanf("%d", &newStudent.age);
    clearInputBuffer();

    printf("Enter student course: ");
    fgets(newStudent.subjects, MAX_COURSE_LENGTH, stdin);
    newStudent.subjects[strcspn(newStudent.subjects, "\n")] = '\0';

    printf("Enter grades for given subjects:\n");
    for (int i = 0; i < 7; i++) {
        printf("Subject %d: ", i+1);
        scanf("%f", &newStudent.grades[i]);
        clearInputBuffer();
    }

    calculateAverage(&newStudent);
    students[studentsCount++] = newStudent;
    printf("Successfully!\n");
}

void calculateAverage(Student *student) {
    float sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += student->grades[i];
    }
    student->average = sum / 5;
}

void displayStudents() {
    if (studentsCount == 0) {
        printf("No students at all\n");
        return;
    }

    printf("\n%-20s %-10s %-5s %-15s %-10s\n",
           "Name", "ID", "Age", "Course", "Average");
    printf("!-!-!-!-!-!-!-!-!-!-!-!-!\n");

    for (int i = 0; i < studentsCount; i++) {
        printf("%-20s %-10d %-5d %-15s %-10.2f\n",
               students[i].name, students[i].id, students[i].age,
               students[i].subjects, students[i].average);
    }
}

void searchStudent() {
    if (studentsCount == 0) {
        printf("No students yet\n");
        return;
    }

    int choice;
    printf("\nSearch options are:\n");
    printf("1. Student ID\n");
    printf("2. Student Name\n");
    printf("3. Courses\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    clearInputBuffer();

    switch(choice) {
        case 1: {
            int id;
            printf("Students ID: ");
            scanf("%d", &id);
            clearInputBuffer();

            int found = 0;
            for (int i = 0; i < studentsCount; i++) {
                if (students[i].id == id) {
                    printf("\nStudent Here!\n");
                    printf("Name: %s\n", students[i].name);
                    printf("ID: %d\n", students[i].id);
                    printf("Age: %d\n", students[i].age);
                    printf("Course: %s\n", students[i].subjects);
                    printf("Grades: ");
                    for (int j = 0; j < 5; j++) {
                        printf("%.2f ", students[i].grades[j]);
                    }
                    printf("\nAverage: %.2f\n", students[i].average);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("Student not found with ID: %d\n", id);
            }
            break;
        }
        case 2: {
            char name[MAX_NAME_LENGTH];
            printf("Enter student name: ");
            fgets(name, MAX_NAME_LENGTH, stdin);
            name[strcspn(name, "\n")] = '\0';

            int found = 0;
            printf("\nSearch results:\n");
            for (int i = 0; i < studentsCount; i++) {
                if (strstr(students[i].name, name) != NULL) {
                    printf("%s (ID: %d)\n", students[i].name, students[i].id);
                    found++;
                }
            }
            if (!found) {
                printf("No students found with name containing: %s\n", name);
            }
            break;
        }
        case 3: {
            char subjects[MAX_COURSE_LENGTH];
            printf("Enter course: ");
            fgets(subjects, MAX_COURSE_LENGTH, stdin);
            subjects[strcspn(subjects, "\n")] = '\0';

            int found = 0;
            printf("\nStudents in course %s:\n", subjects);
            for (int i = 0; i < studentsCount; i++) {
                if (strcmp(students[i].subjects, subjects) == 0) {
                    printf("%s (ID: %d)\n", students[i].name, students[i].id);
                    found++;
                }
            }
            if (!found) {
                printf("No students found in course: %s\n", subjects);
            }
            break;
        }
        default:
            printf("Invalid choice.\n");
    }
}

void updateStudent() {
    if (studentsCount == 0) {
        printf("No students yet\n");
        return;
    }

    int id;
    printf("Enter student ID: ");
    scanf("%d", &id);
    clearInputBuffer();

    int found = -1;
    for (int i = 0; i < studentsCount; i++) {
        if (students[i].id == id) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("Student not found with ID: %d\n", id);
        return;
    }

    printf("\nCurrent student info:\n");
    printf("1. Name: %s\n", students[found].name);
    printf("2. Age: %d\n", students[found].age);
    printf("3. Courses: %s\n", students[found].subjects);
    printf("4. Grades: ");
    for (int i = 0; i < 5; i++) {
        printf("%.2f ", students[found].grades[i]);
    }
    printf("\n");

    printf("Enter field number to update (1-4) or 0 to cancel: ");
    int field;
    scanf("%d", &field);
    clearInputBuffer();

    switch(field) {
        case 0:
            printf("Update cancelled.\n");
            break;
        case 1:
            printf("Enter new name: ");
            fgets(students[found].name, MAX_NAME_LENGTH, stdin);
            students[found].name[strcspn(students[found].name, "\n")] = '\0';
            printf("Name updated successfully.\n");
            break;
        case 2:
            printf("Enter new age: ");
            scanf("%d", &students[found].age);
            clearInputBuffer();
            printf("Age updated.\n");
            break;
        case 3:
            printf("Enter new course: ");
            fgets(students[found].subjects, MAX_COURSE_LENGTH, stdin);
            students[found].subjects[strcspn(students[found].subjects, "\n")] = '\0';
            printf("Course updated.\n");
            break;
        case 4:
            printf("Enter the new grades for 7 subjects:\n");
            for (int i = 0; i < 5; i++) {
                printf("Subject %d: ", i+1);
                scanf("%f", &students[found].grades[i]);
                clearInputBuffer();
            }
            calculateAverage(&students[found]);
            printf("Grades updated.\n");
            break;
        default:
            printf("Invalid\n");
    }
}

void deleteStudent() {
    if (studentsCount == 0) {
        printf("No students to delete.\n");
        return;
    }

    int id;
    printf("Student ID for deletion: ");
    scanf("%d", &id);
    clearInputBuffer();

    int found = -1;
    for (int i = 0; i < studentsCount; i++) {
        if (students[i].id == id) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("Students with seemingly no ID: %d\n", id);
        return;
    }

    printf("Are you sure you want to delete %s (ID: %d)? (y/n): ",
           students[found].name, students[found].id);
    char confirm;
    scanf("%c", &confirm);
    clearInputBuffer();

    if (tolower(confirm) == 'y') {
        for (int i = found; i < studentsCount - 1; i++) {
            students[i] = students[i + 1];
        }
        studentsCount--;
        printf("Student deleted successfully.\n");
    } else {
        printf("Cancelled\n");
    }
}

void displayMenu() {
    printf("\n1. Add Student\n");
    printf("2. Display All Students\n");
    printf("3. Search Student\n");
    printf("4. Update Student\n");
    printf("5. Delete Student\n");
    printf("6. Sort Students\n");
    printf("7. Save and Exit\n");
}

void clearInputBuffer() {
    while (getchar() != '\n');
}

void sortStudents() {
    printf("Sort function not implemented yet\n");
}