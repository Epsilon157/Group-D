//Group D
//Author Keegan Neal
//Keegan.neal@okstate.edu
//04/06/25

#include <stdio.h>
#include <pthread.h>
#include "shared_header.h"
#include <string.h>
//#include "main.c"

/*
    This code includes making sure mutexes were configured correctly, also ensuring that mutexes could work in shared space with all of the trains(processes) 
    allowing to be shared throughout all of the processes safely.I also made sure there was a capacity check for allowing trains (processes) was set to only 1 to ensure 
    it would be compatible with mutexes and also only allowing one train can enter at a time in an intersection.
*/
// method for initializing mutex
void initializeMutex(Intersection *intersections, int intersectionCount){
   pthread_mutexattr_t at;
   pthread_mutexattr_init(&at);
   pthread_mutexattr_setpshared(&at, PTHREAD_PROCESS_SHARED);

   for(int i =0; i < intersectionCount; i++){
    //checking id the resource or capacity is == 1 so the mutex can be initialized
    if(intersections[i].capacity == 1){
        pthread_mutex_init(&intersections[i].Mutex, &at);
        printf("Intersection %s initialized \n", intersections[i].name);
    }//added error handling
    else if(intersections[i].capacity < 0){
        printf("Error: Intersection %s has an invalid resource value: %i (must be greater than 0)", intersections[i].name, intersections[i].capacity);
        exit(0);
    }
   }
    
}

//Acquiring a lock for intersections that can only fit one process
void acquireTrainMutex(Intersection *intersection, const char *trainName){
   //send train in intersection if there is only 1 
    if(strcmp(intersection-> lock_type, "Mutex") == 0){
        pthread_mutex_lock(&intersection ->Mutex);
        intersection -> lock_state =1;
        printf("Intersection %s acquried by Train %s\n", intersection->name, trainName);
    }

}
//Releasin for intersections that can only fit one process by unlocking
void releaseTrainMutex(Intersection *intersection, const char *trainName){
    if(strcmp(intersection-> lock_type, "Mutex") == 1){
        pthread_mutex_unlock(&intersection-> Mutex);
        intersection -> lock_state = 0;
        printf("Intersection %s releasing Train %s\n", intersection->name, trainName);

    }

}
//writing a test to ensure the code mutexes are called correctly

void test_initializeMutex() {
    Intersection intersections[1];
    strcpy(intersections[0].name, "TestIntersection");
    intersections[0].capacity = 2;  // This should SKIP mutex initialization

    initializeMutex(intersections, 1);

    int lockResult = pthread_mutex_lock(&intersections[0].Mutex);

    if (lockResult == 0) {
        printf("ERROR: Mutex was lockable for %s, but it should NOT have been initialized!\n", intersections[0].name);
        pthread_mutex_unlock(&intersections[0].Mutex);
    } else {
        printf("Correct behavior: Mutex not initialized for %s (capacity > 1)\n", intersections[0].name);
    }
}
