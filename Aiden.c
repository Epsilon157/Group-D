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
//#include "Jacob.c"
//#include "main.c"

//Function to initialize the semaphores for every intersection with more than 1 resource
void initializeSemaphores(Intersection **intersections, int intersectionCount){
    for(int i = 0; i < intersectionCount; i++){
        if((*intersections)[i].resources > 1){
            (*intersections)[i].Semaphore = malloc(sizeof(sem_t));
            sem_init(&(*intersections)[i].Semaphore, 0, (*intersections)[i].resources);
        }
        else if((*intersections)[i].resources < 0){
            printf("Error: Intersection %c has an invalid resource value: %i (must be greater than 0)", (*intersections)[i].name, (*intersections)[i].resources);
        }
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
void clearSemaphores(Intersection **intersections, int intersectionCount){
    for(int i = 0; i < intersectionCount; i++){
        if((*intersections)[i].resources > 1){
            sem_destroy(&((*intersections)[i].Semaphore));
            free((*intersections)[i].Semaphore);
        }
    }
}