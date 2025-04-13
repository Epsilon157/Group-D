

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

//Function that makes the resource allocation table
void initR_Table(Intersection **intersections, int intersectionCount){
	
	for(int i = 0; i < intersectionCount; i++){
		
		//initializes lock_state to 0
		(*intersections)[i].lock_state = 0;
		(*intersections)[i].trainCount = 0;
		//Test case
		strcpy((*intersections)[0].trains[0].name, "Train 1");
		strcpy((*intersections)[1].trains[0].name, "Train 2");
		strcpy((*intersections)[1].trains[1].name, "Train 3");

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



