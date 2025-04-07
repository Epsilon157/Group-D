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
typedef struct {
    char name[50];  // Intersection name
    int capacity;  // Resources available at this intersection
    pthread_mutex_t Mutex;
} Intersection;
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
   // else if(intersections[i].resources < 0){
     //   printf("Error: Intersection %c has an invalid resource value: %i (must be greater than 0)", intersections[i].name, (*intersections)[i].resources);
       // exit(0);
    //}
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
