// Group D
// Jacob Abad
// jacob.abad10@okstate.edu@okstate.edu
// 4/5/2025

// Jacob's Week 2 role: Parsing files
// Parsing for both intersections and trains to put into objects

// Jacob's Week 3 role: Simulation Clock and Reallocated work of logging messages.

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  // For va_list, va_start, va_end
#include <pthread.h> // For mutexes
#include "shared_header.h"

// Global variables
int *sim_time = NULL;
pthread_mutex_t sim_time_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for protecting sim_time

// Function to initialize simulation time
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
    if (!format) {
        fprintf(stderr, "logEvent: NULL format string\n");
        return;
    }

    char timeBuffer[10];
    formatTime(timeBuffer);  // Format current time

    va_list args;
    va_start(args, format);

    FILE *out = log_file ? log_file : stderr;

    fprintf(out, "%s ", timeBuffer);
    vfprintf(out, format, args);
    fprintf(out, "\n");

    va_end(args);
}

// Function to print initialized intersections
void print_initialized_intersections(Intersection *intersections, int count) {
    logEvent("SERVER: Initialized intersections:");
    for (int i = 0; i < count; i++) {
        if (intersections[i].capacity == 1)
            logEvent("- %s (Mutex, Capacity=%d)", intersections[i].name, intersections[i].capacity);
        else
            logEvent("- %s (Semaphore, Capacity=%d)", intersections[i].name, intersections[i].capacity);
    }
}

// Function to print request sent for intersection
void printRequestSent(int trainIndex, const char *intersection) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("%s: Sent ACQUIRE request for %s.", trains[trainIndex].name, intersection);
}

// Function to print request release for intersection
void printRequestRelease(int trainIndex, const char *intersection) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("%s: Released %s.", trains[trainIndex].name, intersection);
}

// Function to print denied request for intersection
void printDenied(const char *train, const char *intersection) {
    const char *safeTrain = train ? train : "(null)";
    const char *safeIntersection = intersection ? intersection : "(null)";

    logEvent("SERVER: %s is locked. %s added to wait queue.", safeIntersection, safeTrain);
}

void ForceRelease(const char *train, const char *intersection){
    logEvent("SERVER: %s released %s forcibly");
}
// Function to log a deadlock and preemption event
void Deadlock(char train1, char train2, char intersection1) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("SERVER: Deadlock Detected! %s ↔ %s", train1, train2);
    logEvent("SERVER: Preempting %s from %s", intersection1, train1);
    logEvent("SERVER: %s released %s forcibly", train1, intersection1);
}

// Function to print intersection granted for a train
void printIntersectionGranted(int trainIndex, const char *intersectionName) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    int i;
    int matchIndex = -1;

    // Find the matching intersection
    for (i = 0; i < 100; i++) {
        if (intersections[i].name == NULL) continue; // skip uninitialized entries
        if (strcmp(intersections[i].name, intersectionName) == 0) {
            matchIndex = i;
            break;
        }
    }

    if (matchIndex == -1) {
        logEvent("SERVER: ERROR - Intersection %s not found for train %s", intersectionName, trains[trainIndex].name);
        //fprintf(log_file, "\n");
        return;
    }
    if (strcmp(intersections[matchIndex].lock_type, "Semaphore") == 0) {
        int semValue;
        sem_getvalue(&intersections[matchIndex].Semaphore, &semValue);
    
        logEvent("SERVER: GRANTED %s to %s. Semaphore count: %d.",intersectionName, trains[trainIndex].name, semValue);
        logEvent("%s: Acquired %s. Proceeding...", trains[trainIndex].name, intersectionName);
    }
     else {
        logEvent("SERVER: GRANTED %s to %s.", intersectionName, trains[trainIndex].name, intersections[matchIndex].capacity);
        logEvent("%s: Acquired %s. Proceeding...", trains[trainIndex].name, intersectionName);
    }

    //fprintf(log_file, "\n");
    
}

// Function to log a deadlock and preemption event
void printDeadlockDetected1(const char *train1, const char *train2, const char *intersection) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("SERVER: Deadlock detected! Cycle: %s ? %s.", train1, train2);
    //logEvent("SERVER: Preempting %s from %s.", intersection, train1);
    //logEvent("SERVER: %s released %s forcibly.", train1, intersection);
}



void printDeadlockDetected(char *deadlockedTrains[], int deadlockedCount) {
    // Start the log message with the "Deadlock detected!" message
    char logMessage[1024];  // Adjust size as needed to fit all train names

    // Format the initial part of the log message
    snprintf(logMessage, sizeof(logMessage), "SERVER: Deadlock detected! Cycle: %s", deadlockedTrains[0]);

    // Append the rest of the trains in the cycle
    for (int i = 1; i < deadlockedCount; i++) {
        // Concatenate " ↔ " and the next train name
        strncat(logMessage, " ↔ ", sizeof(logMessage) - strlen(logMessage) - 1);
        strncat(logMessage, deadlockedTrains[i], sizeof(logMessage) - strlen(logMessage) - 1);
    }

    // Log the entire cycle in one line
    strncat(logMessage, ".", sizeof(logMessage) - strlen(logMessage) - 1);
    logEvent("%s", logMessage);
}

void AttemptingDeadlockResolve(const char *intersectionName, const char *victim) {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("SERVER: Preempting %s from %s.", intersectionName, victim);
    //logEvent("SERVER: Preempting %s from %s.", intersection, train1);
    //logEvent("SERVER: %s released %s forcibly.", train1, intersection);
}


// Function to log the completion of the simulation
void printSimulationComplete() {
    //pthread_mutex_lock(&sim_time_mutex); // Lock mutex to safely increment sim_time
    //(*sim_time)++;  // Increment simulation time
   // pthread_mutex_unlock(&sim_time_mutex) // Unlock mutex after incrementing sim_time

    logEvent("SIMULATION COMPLETE. All trains reached destinations.");
}

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
