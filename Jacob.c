#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared_header.h"

/*
// Structure to hold data about intersections
typedef struct {
    char name[50];  // Intersection name
    int capacity;   // Capacity available at this intersection
} Intersection;

// Structure to hold data about trains
typedef struct {
    char name[50];  // Train name (e.g., Train1)
    char **route;   // Array of intersection names the train passes through
    int routeCount; // Number of intersections the train visits
} Train;
*/
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
        int capacity;  // Changed from 'resources' to 'capacity'
        sscanf(line, "%[^:]:%d", name, &capacity);  // Changed from 'resources' to 'capacity'
        strcpy((*intersections)[i].name, name);
        (*intersections)[i].capacity = capacity;  // Changed from 'resources' to 'capacity'
        i++;
    }

    fclose(file);
    return count;
}

// Function to parse the train data
// Function to parse the train data
int TrainParsing(const char *filename, Train **trains) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    int numberOfTrains = 0;  // Changed the variable name to 'numberOfTrains'
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        numberOfTrains++;  // Increment count for each train
    }
    rewind(file);

    *trains = malloc(numberOfTrains * sizeof(Train));

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
    return numberOfTrains;  // Return the number of trains
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


void GetIntersectionCapacity(Intersection *intersections, int intersectionCount, int intersectionIndex) {
    if (intersectionIndex >= 1 && intersectionIndex <= intersectionCount) {
        printf("Capacity at intersection %s: %d\n", intersections[intersectionIndex - 1].name, intersections[intersectionIndex - 1].capacity);  // Changed 'resources' to 'capacity'
    } else {
        printf("Invalid intersection number.\n");
    }
}



//Week 3 code


char* updateAndFormatTime(char* buffer) {
    // Initialize variables
    static int var1 = 0, var2 = 0, var3 = 0;

    // Increment var1
    var1++;

    // If var1 reaches 60, reset it to 0 and increment var2
    if (var1 >= 60) {
        var1 = 0;
        var2++;
    }

    // If var2 reaches 60, reset it to 0 and increment var3
    if (var2 >= 60) {
        var2 = 0;
        var3++;
    }

    // Format the time string as [hh:mm:ss]
    sprintf(buffer, "[%02d:%02d:%02d]", var3, var2, var1);

    return buffer;
}



void printMessages(char* train, char* intersection, int granted) {
    char timeBuffer[10]; // Buffer to hold the time string
    updateAndFormatTime(timeBuffer);  // Update and get the formatted time

    if (granted == 1) {
        // If granted is 1, print the granted message
        printf("%s %s: Sent ACQUIRE request for %s.\n", timeBuffer, train, intersection);
        printf("%s SERVER: GRANTED %s to %s.\n", timeBuffer, intersection, train);
    } else {
        // If granted is 0, print the denied message
        printf("%s %s: Sent ACQUIRE request for %s.\n", timeBuffer, train, intersection);
        printf("%s SERVER: %s is locked. %s added to wait queue.\n", timeBuffer, intersection, train);
    }
}




void print_initialized_intersections(Intersection *intersections, int num_intersections) {
    printf("00:00:00] SERVER: Initialized intersections:\n");

    for (int i = 0; i < num_intersections; i++) {
        // Check if the intersection's capacity is 1 (Mutex) or greater than 1 (Semaphore)
        if (intersections[i].capacity == 1) {
            printf("- %s (Mutex, Capacity=%d)\n", 
                   intersections[i].name, 
                   intersections[i].capacity);
        } else {
            printf("- %s (Semaphore, Capacity=%d)\n", 
                   intersections[i].name, 
                   intersections[i].capacity);
        }
    }
}
