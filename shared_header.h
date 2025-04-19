// shared_structs.h
#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <pthread.h>
#include <semaphore.h>

// Enumerations for train action and server response types
typedef enum {
    ACQUIRE,
    RELEASE,
    FINISHED
} TrainAction;

typedef enum {
    GRANT,
    WAIT,
    DENY
} ServerResponse;

// Structure for message queue
typedef struct msg_buffer {
    long msg_type;      // Message type (used for routing)
    int trainIndex;
    char intersectionName[50];
    int action;
    int response;
} Message;

// Graph data structure elements needed for resource
// allocation graph generation (digraph)
typedef struct Edge {
    char target[50];
    struct Edge *next;
} Edge;

typedef struct Node {
    char name[50];
    int isTrain;         // 1 = Train, 0 = Intersection
    Edge *edges;         // Outgoing edges
    struct Node *next;   // Next node in graph
} Node;

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
    int isMutexInitialized;
    pthread_mutex_t Mutex;
    sem_t Semaphore;
} Intersection;

void printRequestSent(int trainIndex, const char *intersection);
void printIntersectionGranted(int trainIndex, const char *intersectionName);
void AttemptingDeadlockResolve(const char *intersectionName, const char *victim);
void printRequestRelease(int trainIndex, const char *intersection);
void printSimulationComplete();
void printDenied(int trainIndex, const char *intersection);
void ForceRelease(const char *train, const char *intersection);
void freeRAG(Node *head);
void acquireTrainMutex(Intersection *intersection, const char *trainName);
int tryAcquireMutex(Intersection *intersection, const char *trainName);
void releaseTrainMutex(Intersection *intersection, const char *trainName);
void releaseTrain(Intersection *intersection, const char *trainName);
void acquireTrain(Intersection *intersection, const char *trainName);
void printDeadlockDetected(char *deadlockedTrains[], int deadlockedCount);
void resolveDeadlock(Train **trains, int trainCount, Intersection **intersections, int intersectionCount, int msgid);
void listTrainsInDeadlock(Node *RAG);
void mutexAcqu(Intersection *targetIntersection, Train *train, int msgid, int trainIndex, const char *intersectionName);
void semaphoreAcqu(Intersection *targetIntersection, Train *train, int msgid, int trainIndex, const char *intersectionName);
int tryAcquireSemaphore(Intersection *intersection, const char *trainName);
extern int *sim_time;  // Simulation time pointer
extern pthread_mutex_t sim_time_mutex;  // Mutex for controlling sim_time
extern pthread_mutex_t finished_mutex;  
FILE *log_file = NULL;


Intersection *intersections; //From * to **
Train *trains;

#endif
