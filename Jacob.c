#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  // For va_list, va_start, va_end
#include "shared_header.h"

// Global variables for simulation time and log file


// ----------- Utility Functions -----------
int *sim_time = NULL;

void initialize_sim_time() {
    sim_time = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sim_time == MAP_FAILED) {
        perror("mmap failed for sim_time");
        exit(1);
    }
    *sim_time = 0; // Initialize to 0
}
// Function to format the current simulation time
void formatTime(char *buffer) {
    
    int hours = (*sim_time) / 3600;
    int minutes = ((*sim_time) % 3600) / 60;
    int seconds = (*sim_time) % 60;
    
    sprintf(buffer, "[%02d:%02d:%02d]", hours, minutes, seconds);
}

// Function to log events in the simulation
void logEvent(const char *format, ...) {
    char timeBuffer[10];
    formatTime(timeBuffer);  // Format current time

    va_list args;
    va_start(args, format);

    if (log_file) {  // Ensure the log file is open
        fprintf(log_file, "%s ", timeBuffer);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
    }

    va_end(args);
}


// ----------- Simulation Logging -----------

// Function to print initialized intersections to the log file
void print_initialized_intersections(Intersection *intersections, int count) {
    logEvent("SERVER: Initialized intersections:");
    for (int i = 0; i < count; i++) {
        if (intersections[i].capacity == 1)
            logEvent("- %s (Mutex, Capacity=%d)", intersections[i].name, intersections[i].capacity);
        else
            logEvent("- %s (Semaphore, Capacity=%d)", intersections[i].name, intersections[i].capacity);
    }
    
}

// Function to print train messages regarding intersection requests and grants
void printMessages(const char *train, const char *intersection, int granted, int remaining) {
    (*sim_time)++;
    logEvent("%s: Sent ACQUIRE request for %s.", train, intersection);
    if (granted) {
        if (remaining >= 0)
            logEvent("SERVER: GRANTED %s to %s. Semaphore count: %d.", intersection, train, remaining);
        else
            logEvent("SERVER: GRANTED %s to %s.", intersection, train);
    } else {
        logEvent("SERVER: %s is locked. %s added to wait queue.", intersection, train);
    }
}

void printRequestSent(const char *train, const char *intersection){
    (*sim_time)++;
    logEvent("%s: Sent ACQUIRE request for %s.", train, intersection);
    fprintf(log_file, "\n");
}

void printRequestGranted(const char *train, const char *intersection){
        (*sim_time)++;
    logEvent("%s: Sent ACQUIRE request for %s.", train, intersection);
    fprintf(log_file, "\n");
}


// Function to log a deadlock and preemption event
void printDeadlockAndPreemption(const char *train1, const char *train2, const char *intersection) {
    (*sim_time)++;
    logEvent("SERVER: Deadlock detected! Cycle: %s ? %s.", train1, train2);
    logEvent("SERVER: Preempting %s from %s.", intersection, train1);
    logEvent("SERVER: %s released %s forcibly.", train1, intersection);
}

// Function to log the completion of the simulation
void printSimulationComplete() {
    (*sim_time)++;
    logEvent("SIMULATION COMPLETE. All trains reached destinations.");
}

// ----------- Structure Definitions -----------

// Structure to hold data about intersections


// File paths for intersections and trains
const char *intersectionFilePath = "intersections.txt";
const char *trainFilePath = "trains.txt";

// ----------- Parsing Functions -----------

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
        int capacity;
        sscanf(line, "%[^:]:%d", name, &capacity);
        strcpy((*intersections)[i].name, name);
        (*intersections)[i].capacity = capacity;
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

    int numberOfTrains = 0;
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        numberOfTrains++;
    }
    rewind(file);

    *trains = malloc(numberOfTrains * sizeof(Train));

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char name[50];
        char route[200];
        sscanf(line, "%[^:]:%s", name, route);

        // Ensure the name is properly null-terminated and fits in the buffer
        strncpy((*trains)[i].name, name, sizeof((*trains)[i].name) - 1);
        (*trains)[i].name[sizeof((*trains)[i].name) - 1] = '\0';  // Ensure null termination

        (*trains)[i].route = NULL;
        (*trains)[i].routeCount = 0;

        // Parse the route string and allocate memory for each intersection
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
    return numberOfTrains;
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

// Function to get the capacity of a specific intersection
void GetIntersectionCapacity(Intersection *intersections, int intersectionCount, int intersectionIndex) {
    if (intersectionIndex >= 1 && intersectionIndex <= intersectionCount) {
        printf("Capacity at intersection %s: %d\n", intersections[intersectionIndex - 1].name, intersections[intersectionIndex - 1].capacity);
    } else {
        printf("Invalid intersection number.\n");
    }
}
