/* Add text options when ran so anyone can find how to pull the data out.
 As we get more code for the data to be used in I will adjust parsing so it returns the 
 info needed*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold data about intersections
typedef struct {
    char name[50];  // Intersection name
    int resources;  // Resources available at this intersection
} Intersection;

// Structure to hold data about trains
typedef struct {
    char name[50];  // Train name (e.g., Train1)
    char **route;   // Array of intersection names the train passes through
    int routeCount; // Number of intersections the train visits
} Train;

// File paths for intersections and trains
const char *intersectionFilePath = "intersections.txt";
const char *trainFilePath = "trains.txt";

// Function to parse the intersection data
int IntersectionParsing(const char *filename, Intersection **intersections) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    rewind(file);

    *intersections = malloc(count * sizeof(Intersection));

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char name[50];
        int resources;
        sscanf(line, "%[^:]:%d", name, &resources);
        strcpy((*intersections)[i].name, name);
        (*intersections)[i].resources = resources;
        i++;
    }

    fclose(file);
    return count;
}

// Function to parse the train data
int TrainParsing(const char *filename, Train **trains) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    int count = 0;
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    rewind(file);

    *trains = malloc(count * sizeof(Train));

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char name[50];
        char route[200];
        sscanf(line, "%[^:]:%s", name, route);

        strcpy((*trains)[i].name, name);

        (*trains)[i].route = NULL;
        (*trains)[i].routeCount = 0;

        char *token = strtok(route, ",");
        while (token != NULL) {
            (*trains)[i].route = realloc((*trains)[i].route, ((*trains)[i].routeCount + 1) * sizeof(char *));
            (*trains)[i].route[(*trains)[i].routeCount] = malloc(strlen(token) + 1);
            strcpy((*trains)[i].route[(*trains)[i].routeCount], token);
            (*trains)[i].routeCount++;
            token = strtok(NULL, ",");
        }
        i++;
    }

    fclose(file);
    return count;
}

// Function to free the memory allocated for intersections and trains
void FreeMemory(Intersection *intersections, int intersectionCount, Train *trains, int trainCount) {
    free(intersections);

    for (int i = 0; i < trainCount; i++) {
        for (int j = 0; j < trains[i].routeCount; j++) {
            free(trains[i].route[j]);
        }
        free(trains[i].route);
    }
    free(trains);
}

// Function to display the path of a specific train
void DisplayTrainPath(Train *trains, int trainCount, const char *trainName) {
    int found = 0;
    for (int i = 0; i < trainCount; i++) {
        if (strcmp(trains[i].name, trainName) == 0) {
            found = 1;
            printf("Train %s's path: ", trains[i].name);
            for (int j = 0; j < trains[i].routeCount; j++) {
                printf("%s", trains[i].route[j]);
                if (j < trains[i].routeCount - 1) {
                    printf(", ");
                }
            }
            printf("\n");
            break;
        }
    }

    if (!found) {
        printf("Train %s not found.\n", trainName);
    }
}

// Function to get the resource amount for a specific intersection
void GetIntersectionResources(Intersection *intersections, int intersectionCount, int intersectionIndex) {
    if (intersectionIndex >= 1 && intersectionIndex <= intersectionCount) {
        printf("Resources at intersection %s: %d\n", intersections[intersectionIndex - 1].name, intersections[intersectionIndex - 1].resources);
    } else {
        printf("Invalid intersection number.\n");
    }
}
/*
int main() {
    Intersection *intersections;
    Train *trains;

    int intersectionCount = IntersectionParsing(intersectionFilePath, &intersections);
    printf("Parsed %d intersections:\n", intersectionCount);
    for (int i = 0; i < intersectionCount; i++) {
        printf("%s has %d resources\n", intersections[i].name, intersections[i].resources);
    }

    int trainCount = TrainParsing(trainFilePath, &trains);
    printf("\nParsed %d trains:\n", trainCount);
    for (int i = 0; i < trainCount; i++) {
        printf("%s passes through:", trains[i].name);
        for (int j = 0; j < trains[i].routeCount; j++) {
            printf(" %s", trains[i].route[j]);
            if (j < trains[i].routeCount - 1) {
                printf(",");
            }
        }
        printf("\n");
    }

    // Main menu for user to choose
    int choice;
    printf("\nSelect an option:\n");
    printf("1. Select an intersection and find all trains passing through it\n");
    printf("2. Select a stop number (1-%d) and find all trains with that stop\n", trainCount);
    printf("3. Get the path of a specific train\n");
    printf("4. Select an intersection and get the resource amount\n");
    printf("Enter your choice (1, 2, 3, or 4): ");
    scanf("%d", &choice);

    if (choice == 1) {
        int stopNumber;
        printf("Enter the intersection number (1-%d): ", intersectionCount);
        scanf("%d", &stopNumber);

        if (stopNumber >= 1 && stopNumber <= intersectionCount) {
            const char *intersectionName = intersections[stopNumber - 1].name;
            printf("Trains passing through %s:\n", intersectionName);
            for (int i = 0; i < trainCount; i++) {
                for (int j = 0; j < trains[i].routeCount; j++) {
                    if (strcmp(trains[i].route[j], intersectionName) == 0) {
                        printf("%s\n", trains[i].name);
                        break;
                    }
                }
            }
        } else {
            printf("Invalid intersection number.\n");
        }
    } else if (choice == 2) {
        int stopNumber;
        printf("Enter the stop number (1-%d): ", trainCount);
        scanf("%d", &stopNumber);

        printf("Trains at stop %d:\n", stopNumber);
        for (int i = 0; i < trainCount; i++) {
            if (stopNumber <= trains[i].routeCount) {
                printf("%s: %s (Stop %d)\n", trains[i].name, trains[i].route[stopNumber - 1], stopNumber);
            }
        }
    } else if (choice == 3) {
        char trainName[50];
        printf("Enter the train name: ");
        scanf("%s", trainName);
        DisplayTrainPath(trains, trainCount, trainName);
    } else if (choice == 4) {
        int intersectionNumber;
        printf("Enter the intersection number (1-%d): ", intersectionCount);
        scanf("%d", &intersectionNumber);

        GetIntersectionResources(intersections, intersectionCount, intersectionNumber);
    } else {
        printf("Invalid choice.\n");
    }

    FreeMemory(intersections, intersectionCount, trains, trainCount);

    return 0;
}*/
