// Group D
// Zachary SchroederS
// zachary.schroeder@okstate.edu
// 4/19/2025

/*
The purpose of this code is to initialize the resource allocation table and to print it as an output with the function initR_Table()
and with the function printR_Table. The rest of the functions resetTrain(), resetIntersection(), cleanupAll(), and freeRAG() are for 
cleaning up any memory at the end of the program that was allocated during the start up phase of the program. There is also a resolve deadlock
function that preemptively decides which train should give up its intersection.
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "shared_header.h"

//Function that makes the resource allocation table
void initR_Table(Intersection **intersections, int intersectionCount){
	
	for(int i = 0; i < intersectionCount; i++){
		
		//initializes lock_state to 0
		(*intersections)[i].lock_state = 0;

		//initializes lock type to semaphore if its resource > 1 and mutex if else
		if((*intersections)[i].capacity > 1){
			strcpy((*intersections)[i].lock_type, "Semaphore");
		} else{
			strcpy((*intersections)[i].lock_type, "Mutex");
		}

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

//Function that free's trains memory
void resetTrain(Train *train) {
    if (train->route != NULL) {
        for (int j = 0; j < train->routeCount; ++j) {
            if (train->route[j] != NULL) {
                free(train->route[j]);
            }
        }
        free(train->route);
        train->route = NULL;
    }

    // Reset other variables
    for (int j = 0; j < train->heldIntersectionCount; j++) {
        train->heldIntersections[j] = NULL;
    }
    train->waitingIntersection = NULL;
    train->heldIntersectionCount = 0;
    train->routeCount = 0;
    memset(train->name, 0, sizeof(train->name));
}


//Function that resets everything in the intersection object
void resetIntersection(Intersection *intersection) {
    // Get rid of mutex if mutex exists
    if (intersection->isMutexInitialized) {
        pthread_mutex_destroy(&intersection->Mutex);
        intersection->isMutexInitialized = 0;
    }

    // Get rid of semaphore
    sem_destroy(&intersection->Semaphore);

    // Reset variables in intersection object
    memset(intersection->name, 0, sizeof(intersection->name));
    memset(intersection->type, 0, sizeof(intersection->type));
    memset(intersection->lock_type, 0, sizeof(intersection->lock_type));
    intersection->capacity = 0;
    intersection->lock_state = 0;
    intersection->forcedRelease = 0;

    // Reset trains in intersection object
    for (int i = 0; i < 50; i++) {
        resetTrain(&intersection->trains[i]);
    }
}

//Function that cleans up all trains and intersections memory
void cleanupAll(Train *trains, int trainCount, Intersection *intersections, int intersectionCount) {
    for (int i = 0; i < trainCount; i++) {
        resetTrain(&trains[i]);
    }

    for (int i = 0; i < intersectionCount; i++) {
        resetIntersection(&intersections[i]);
    }

    //frees any dynamic memory
     free(trains);
     free(intersections);
}

//Free's RAGs allocated memory
void freeRAG(Node* head) {
    Node* currNode = head;
    while (currNode != NULL) {
        Node* nextNode = currNode->next;

        // Free all edges 
        Edge* edge = currNode->edges;
        while (edge != NULL) {
            Edge* nextEdge = edge->next;
            free(edge);
            edge = nextEdge;
        }
        // Only current node needs to be freed
        free(currNode); 
        currNode = nextNode;
    }
}