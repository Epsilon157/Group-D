

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "shared_header.h"
/*
// Structure to hold data about trains
typedef struct {
    char name[50];  // Train name (e.g., Train1)
    char **route;   // Array of intersection names the train passes through
    int routeCount; // Number of intersections the train visits
} Train;

//Structure that holds data about intersections
typedef struct {
    char name[50];  // Intersection name
	char type[50]; // Type of Intersection
	char lock_type[50]; //Type of lock
    int resources;  // Resources available at this intersection
	int lock_state; //0 is for free, 1 is for locked
	Train trains[50]; //Trains in intersection
	
	pthread_mutex_t Mutex;
	sem_t Semaphore;
	
} Intersection;

*/
//Intersection *intersections;
//Test
//Function that makes the resource allocation table
void initR_Table(Intersection **intersections, int intersectionCount){
	
	for(int i = 0; i < intersectionCount; i++){
		
		//initializes lock_state to 0
		(*intersections)[i].lock_state = 0;
		//Test case

		//initializes lock type to semaphore if its resource > 1 and mutex if else
		if((*intersections)[i].capacity > 1){
			strcpy((*intersections)[i].lock_type, "Semaphore");
		} else{
			strcpy((*intersections)[i].lock_type, "Mutex");
		}

		//will implement aiden and keegan code to initialize 
		/*
		if(strcmp((*intersections)[i].lock_type, "Semaphore")){
			sem_init(&(*intersections)[i].Semaphore, 0, (*intersections)[i].capacity);
		} else{
			pthread_mutex_init(&(*intersections)[i].Mutex, NULL);
		}
		
		*/
	} 
	
}

//Function that prints resource allocation table
void printR_Table(Intersection **intersections, int intersectionCount) {
    printf("Resource Allocation Table:\n");
    printf("---------------------------------------------------------------\n");
    printf("IntersectionID | Type | Capacity | Lock State | Holding Trains\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < intersectionCount; i++) {
        printf("%-15s | %-9s | %-8d | %-10s |", (*intersections)[i].name,
               strcmp((*intersections)[i].lock_type, "Semaphore") ? "Mutex" : "Semaphore",
               (*intersections)[i].capacity, ((*intersections)[i].lock_state == 0) ? "Free" : "Locked");
		int j = 0;
		if(strcmp((*intersections)[i].trains[0].name, "") == 0){
			printf(" None");
		}else {
        while(!(strcmp((*intersections)[i].trains[j].name, "") == 0)){
			printf("%-1s ", (*intersections)[i].trains[j].name);
			j++;
			}
		}

        printf("\n");
    }
    printf("---------------------------------------------------------------\n");
}

void resetTrain(Train *train) {
    // Free each dynamically allocated string in the route
    if (train->route != NULL) {
        for (int j = 0; j < train->routeCount; ++j) {
            if (train->route[j] != NULL) {
                free(train->route[j]);
            }
        }
        free(train->route);
        train->route = NULL;
    }

    // Reset other fields
    for (int j = 0; j < train->heldIntersectionCount; j++) {
        train->heldIntersections[j] = NULL;
    }
    train->waitingIntersection = NULL;
    train->heldIntersectionCount = 0;
    train->routeCount = 0;
    memset(train->name, 0, sizeof(train->name));
}

void resetIntersection(Intersection *intersection) {
    // Destroy mutex if initialized
    if (intersection->isMutexInitialized) {
        pthread_mutex_destroy(&intersection->Mutex);
        intersection->isMutexInitialized = 0;
    }

    // Destroy semaphore
    sem_destroy(&intersection->Semaphore);

    // Reset fields
    memset(intersection->name, 0, sizeof(intersection->name));
    memset(intersection->type, 0, sizeof(intersection->type));
    memset(intersection->lock_type, 0, sizeof(intersection->lock_type));
    intersection->capacity = 0;
    intersection->lock_state = 0;
    intersection->forcedRelease = 0;

    // Reset the embedded trains
    for (int i = 0; i < 50; i++) {
        resetTrain(&intersection->trains[i]);
    }
}

void cleanupAll(Train *trains, int trainCount, Intersection *intersections, int intersectionCount) {
    for (int i = 0; i < trainCount; i++) {
        resetTrain(&trains[i]);
    }

    for (int i = 0; i < intersectionCount; i++) {
        resetIntersection(&intersections[i]);
    }

    // Optional: free arrays if dynamically allocated
    // free(trains);
    // free(intersections);
}

