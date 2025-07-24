#include <stdio.h>

#define MAX_STUDENTS 100
#define MAX_NAME_LENGTH 50
#define MAX_COURSE_LENGTH 30
#define FILENAME "students.txt"//will change this later maybe

typedef struct {
    char name[MAX_NAME_LENGTH];
    int id;
    int age;
    char subjects[MAX_COURSE_LENGTH];
    float grades[15]; //Incase they have 15 subjects.
    float average;
} Student;

Student students[MAX_STUDENTS];
int studentsCount = 0;

//Declaring ALL these function prototypes. 1st step.
void loadStudents();
void saveStudents();
void addStudent();
void displayStudents();
void searchStudent();
void updateStudent();
void deleteStudent();
void sortStudents();
void calculateAverage(Student *student);//Mathematically makes sense
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