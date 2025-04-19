//Group D
//Aiden Maner
//aiden.maner@okstate.edu
//4/3/2025
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "shared_header.h"

//Function to initialize the semaphores for every intersection with more than 1 resource
void initializeSemaphores(Intersection **intersections, int intersectionCount){
    // Iterate through the intersections
    for (int i = 0; i < intersectionCount; i++) {
        // Check if the intersection capacity is greater than 1
        if ((*intersections)[i].capacity > 1) {
            // Initialize the semaphore for the intersection
            sem_init(&(*intersections)[i].Semaphore, 0, (*intersections)[i].capacity);
            printf("Semaphore Intersection %s initialized \n", (*intersections)[i].name);
            printf("Intersection %s initialized with lock type: %s and capacity: %d\n", (*intersections)[i].name, (*intersections)[i].lock_type, (*intersections)[i].capacity);
        }
        // Error check for invalid resources (optional, uncomment if needed)
        // else if ((*intersections)[i].resources < 0) {
        //    printf("Error: Intersection %c has an invalid resource value: %i (must be greater than 0)\n", (*intersections)[i].name, (*intersections)[i].resources);
        //    exit(0);
        // }
    }
}
//Function for a train to try and go through an intersection
//Takes the desired intersection and the name of the train wanting to go through
//-----Both of these need to be combined with Keegan's-----
void acquireTrain(Intersection *intersection, const char *trainName){
    if((*intersection).lock_type == "Semaphore"){
        sem_wait(&((*intersection).Semaphore));
        printf("Intersection %s acquired by Train %s\n", intersection->name, trainName);
    }
}
//Function for a train to clear an intersection
//Takes the desired intersection and the name of the train wanting to go through
void releaseTrain(Intersection *intersection, const char *trainName){
    if((*intersection).lock_type == "Semaphore"){
        sem_post(&((*intersection).Semaphore));
        printf("Intersection %s releasing Train %s\n", intersection->name, trainName);
    }
}
//Function to destroy the semaphores and free them from memory
void clearSemaphores(Intersection *intersections, int intersectionCount){
    for(int i = 0; i < intersectionCount; i++){
        if(intersections[i].capacity > 1){
            sem_destroy(&(intersections[i].Semaphore));
          //  free((*intersections)[i].Semaphore);
        }
    }
}
