// shared_structs.h
#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char name[50];  // Train name (e.g., Train1)
    char **route;   // Array of intersection names the train passes through
    char *heldIntersections[50]; // intersections trains hold
    char *waitingIntersection; // intersection train is waiting for 
    int heldIntersectionCount;  // how many intersecions the train holds
    int routeCount;
} Train;

typedef struct {
    char name[50];         // Intersection name
    char type[50];         // Type of intersection
    char lock_type[50];    // "Mutex" or "Semaphore"
    int capacity;         // Capacity
    int lock_state;        // 0 = free, 1 = locked
    Train trains[50];      // Trains currently in intersection
    pthread_mutex_t Mutex;
    sem_t Semaphore;
} Intersection;

void printRequestSent(int trainIndex, const char *intersection);
void printIntersectionGranted(int trainIndex, const char *intersectionName);

void printRequestRelease(int trainIndex, const char *intersection);



int tryAcquireMutex(Intersection *intersection, const char *trainName);
void releaseTrainMutex(Intersection *intersection, const char *trainName);
extern int *sim_time;
FILE *log_file = NULL;

Intersection *intersections; //From * to **
Train *trains;

#endif
