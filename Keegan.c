//Group D
//Author Keegan Neal
//Keegan.neal@okstate.edu
//04/06/25

#include <stdio.h>
#include <pthread.h>
#include "shared_header.h"
#include <string.h>
#include <errno.h>
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
        printf("MUTEX Intersection %s initialized \n", intersections[i].name);
        printf("Intersection %s initialized with lock type: %s and capacity: %d\n", intersections[i].name, intersections[i].lock_type, intersections[i].capacity);

    }
   }
    
}

//Acquiring a lock for intersections that can only fit one process
void acquireTrainMutex(Intersection *intersection, const char *trainName){
   //send train in intersection if there is only 1 
    if(strcmp(intersection-> lock_type, "Mutex") == 0 && intersection->lock_state == 0){
        pthread_mutex_lock(&intersection ->Mutex);
        intersection -> lock_state =1;
        printf("Mutex Intersection %s acquried by %s\n", intersection->name, trainName);
    }

}
// using a tryAcquireMutex function to allow acquiring trains without blocking anything.
int tryAcquireMutex(Intersection *intersection, const char *trainName) {
    if (strcmp(intersection->lock_type, "Mutex") == 0) {
        int result = intersection->lock_state;
        if (result == 0) {
            intersection->lock_state = 1;
            printf("Train %s has acquired Mutex at %s\n", trainName, intersection->name);
            return 0;
        } else if (result == EBUSY) {
            printf("Train %s could not acquire Mutex at %s (Mutex is already locked)\n", trainName, intersection->name);
            return -1;
        } else {
            printf("Train %s failed to acquire Mutex at %s due to error: %d\n", trainName, intersection->name, result);
            return -1;
        }
    }
    return -1;
}

//Releasin for intersections that can only fit one process by unlocking
void releaseTrainMutex(Intersection *intersection, const char *trainName){
    if(strcmp(intersection-> lock_type, "Mutex") == 0){

        pthread_mutex_unlock(&intersection-> Mutex);
        //lock state being 0 for mutex
        intersection -> lock_state = 0;
        //printing the Mutex intersection and train associated with the mutex
        printf("Mutex Intersection %s releasing %s\n", intersection->name, trainName);
    }
}
//writing a test to ensure the code mutexes are called correctly 
//for capacity of 1

void test_initializeMutex() {
    //zeroing out array 
    Intersection intersections[1] ={0};
    //testig if the capacity is > 1 to cause an invalid case for mutexs
    strcpy(intersections[0].name, "TestIntersection");
    intersections[0].capacity = 2;

    //testing the initalization
    initializeMutex(intersections, 1);
    
      //throwing a check to see if mutex is incorrect
    if (intersections[0].isMutexInitialized) {
        //print out an error if it is initialized incorrectly
        printf("ERROR: Mutex should NOT be initialized for %s with capacity > 1\n", intersections[0].name);
        //print out an correrct if it is not initialized incorrectly... meaning its passes the test/
    } else {
        printf("Correct behavior: Mutex not initialized for %s (capacity > 1)\n", intersections[0].name);
    }
}

// write more tests and comments