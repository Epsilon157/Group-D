/*
Group D
Aiden Maner
aiden.maner@okstate.edu
4/19/2025
Description: This program contains the functions that are used for
semaphore initialization, an intersection aquiring and releasing a train,
clearing the semaphores from memory, as well as a function for listing
trains involved in a deadlock.

*/
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

//Function for listing the trains in a deadlock whenever one is detected
void listTrainsInDeadlock(Node *RAG) {
    char *deadlockedTrains[MAX_DEADLOCKED_TRAINS];
    int deadlockedCount = 0;

    Node *curr = RAG;
    while (curr) {
        if (curr->isTrain && curr->edges != NULL) {
            Edge *e = curr->edges;
            while (e) {
                Node *intersection = getNodeByName(RAG, e->target);
                if (intersection) {
                    Edge *back = intersection->edges;
                    while (back) {
                        Node *holdingTrain = getNodeByName(RAG, back->target);
                        if (holdingTrain && holdingTrain->edges != NULL) {
                            // Check if already added
                            int alreadyAdded = 0;
                            for (int i = 0; i < deadlockedCount; i++) {
                                if (strcmp(deadlockedTrains[i], curr->name) == 0) {
                                    alreadyAdded = 1;
                                    break;
                                }
                            }
                            if (!alreadyAdded && deadlockedCount < MAX_DEADLOCKED_TRAINS) {
                                deadlockedTrains[deadlockedCount] = malloc(NAME_LEN);
                                strncpy(deadlockedTrains[deadlockedCount], curr->name, NAME_LEN - 1);
                                deadlockedTrains[deadlockedCount][NAME_LEN - 1] = '\0';
                                deadlockedCount++;
                            }
                            goto next;
                        }
                        back = back->next;
                    }
                }
                e = e->next;
            }
        }
    next:
        curr = curr->next;
    }

    log_file = fopen("simulation.log", "a");
    printDeadlockDetected(deadlockedTrains, deadlockedCount);
    fclose(log_file);

    // Print trains involved in deadlock
    printf("Trains involved in deadlock:\n");
    for (int i = 0; i < deadlockedCount; i++) {
        printf(" - %s\n", deadlockedTrains[i]);
        free(deadlockedTrains[i]);  // Clean up
    }
}

//Function for a train aquiring an intersection with a semaphore
void semaphoreAcqu(Intersection *targetIntersection, Train *train, int msgid, int trainIndex, const char *intersectionName){
    if(sem_trywait(&targetIntersection->Semaphore) == 0){
        // grant the request to the train
        log_file = fopen("simulation.log", "a");
        printIntersectionGranted(trainIndex, intersectionName);
        fclose(log_file);
        printf("Semaphore  %s acquried by %s\n", targetIntersection->name, train->name);
        serverResponse(GRANT, msgid, trainIndex, intersectionName);
        // update train to be holding the intersection
        train->heldIntersections[train->heldIntersectionCount] = strdup(intersectionName); // safe string copy
        train->heldIntersectionCount++;

        // train is no longer waiting on this intersection, so remove it from the waiting list
        free(train->waitingIntersection);
        train->waitingIntersection = NULL; // Clear it after successful grant
    } else {
        int sval;
        if (sem_getvalue(&targetIntersection->Semaphore, &sval) == 0) {
            printf("Semaphore for  %s is busy (value: %d).  %s cannot acquire it.\n",
                targetIntersection->name, sval, train->name);
                log_file = fopen("simulation.log", "a");
                printDenied(trainIndex, intersectionName);
                fclose(log_file);
        } else {
            // If sem_getvalue fails, it's an error (semaphore might be uninitialized or corrupted)
            perror("Error checking semaphore value");
        }
        // If the train can't obtain a mutex or semaphore (if the intersection is full)
        printf("Train%d can't obtain %s, sending WAIT\n", trainIndex + 1, intersectionName);
        train->waitingIntersection = strdup(intersectionName);
        serverResponse(WAIT, msgid, trainIndex, intersectionName);
    }
}
