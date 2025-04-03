// Author Keegan Neal

#include  <stdio.h>
#include <pthread.h>


// method for initializing mutex
void initializeMutex(Intersection *intersections, int intersectionCount){
   pthread_mutexattr_t at;
   pthread_mutexattr_init(&at);
   pthread_mutexattr_setpshared(&at, PTHREAD_PROCESS_SHARED);

   for(int i =0; i < intersectionCount; i++){
    //checking id the resource or capacity is == 1 so the mutex can be initialized
    if(intersections[i].resources == 1){
        pthread_mutex_init(&intersections[i].mutex, &at);
        printf("Intersection %s initialized \n", intersections[i].name);
    }
   }
    
}

//Acquiring a lock for intersections that can only fit one process
void acquireTrain(Intersection *intersection, const char *trainName){
   //send train in intersection if there is only 1 
    if(intersection-> resources == 1){
        pthread_mutex_lock(&intersection ->mutex);
        printf("Intersection %s acquried by Train %s\n", intersection->name, trainName);
    }

}
//Releasin for intersections that can only fit one process by unlocking
void releaseTrain(Intersection *intersection, const char *trainName){
    if(intersection->resources ==1){
        pthread_mutex_unlock(&intersection-> mutex);
        printf("Intersection %s releasing Train %s\n", intersection->name, trainName);

    }

}

