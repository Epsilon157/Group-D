/*
Group D
4/19/2025
Description: This is the main file of the whole project.
It executes all of the functions defined in each group
members' file, while are all included in lines 22
through 27.

To test this project, simple navigate to the Group-D 
folder, compile this file, and execute it using the 
following commands:

gcc -o main main.c
./main
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "shared_header.h"
#include "Cory.c"
#include "Keegan.c" 
#include "Aiden.c"
#include "Jacob.c"
#include "Zack.c"
// #include "Fawaz.c"

int main() {
    initialize_sim_time();
    // create key and message queue ID needed for the
    // message queue to work
    key_t key;
    int msgid;
    //below, should it be & or * or **?
    int intersectionCount = IntersectionParsing(intersectionFilePath, &intersections);
    //adding error handling for file parsing
    if(intersectionCount <= 0){
        fprintf(stderr," ERROR: No intersections were parsed and file could not be read. Exiting \n");
        exit(EXIT_FAILURE);
    }
    printf("Parsed %d intersections:\n", intersectionCount);
    for (int i = 0; i < intersectionCount; i++) {
        printf("%s has %d resources\n", intersections[i].name, intersections[i].capacity);
    }
      // removed &
    int trainCount = TrainParsing(trainFilePath, &trains);
    // added error handling for train parsing
    if(trainCount <=0){
        fprintf(stderr,"ERROR: No trains were parsed. Exiting\n");
        free(intersections);
        exit(EXIT_FAILURE);
    }
    printf("\nParsed %d trains:\n", trainCount);
    for (int i = 0; i < trainCount; i++) {
        printf("%s passes through:", trains[i].name);
        for (int j = 0; j < trains[i].routeCount; j++) {
            printf(" %s", trains[i].route[j]);
            if (j < trains[i].routeCount - 1) {
                printf(",");
            }
        }
        printf("\n");
    }
   
    if (intersectionCount > 0) {
        // Print the initialized intersections
        print_initialized_intersections(intersections, intersectionCount);
        
    } else {
        printf("No intersections to display.\n");
    }
    log_file = fopen("simulation.log", "w");
    if (log_file == NULL) {
        perror("Failed to open simulation.log");
        exit(EXIT_FAILURE);
    }
       
    print_initialized_intersections(intersections, intersectionCount);
    fprintf(log_file, "\n");
    (*sim_time)++;
    int grantTest = 1;
    //printMessages(trains[0].name, intersections[0].name, grantTest, intersections[0].capacity);
    if (log_file) {
        fclose(log_file);
    }

    initR_Table(&intersections, intersectionCount);
    printR_Table(&intersections, intersectionCount);
    initializeSemaphores(&intersections, intersectionCount);//added
    initializeMutex(&intersections, intersectionCount);//added
    //cleanupAll(trains, trainCount, intersections, intersectionCount);
    // This is just testing mutexes are properly working
    printf("\n TEST FOR MUTEX \n") ;
    //test_initializeMutex();
    printf("\n END OF TEST FOR MUTEX \n") ;

    // Create message queue for message passing between parent
    // and child processes. This function MUST return msgid so 
    // that the created message queue ID can be used in the 
    // rest of the main function.
    msgid = createMessageQueue(key, msgid);

    // initial RAG generation
    createRAG_dot(trains, trainCount);
    Node *RAG = createRAG_list(trains, trainCount);
    if (detectCycleInRAG(RAG)) {
        printf("WARNING: Cycle in RAG detected (deadlock can occur)\n");
    } else {
        printf("No cycle in RAG detected\n");
    }
    
    // Fork multiple child processes
    fork_trains(msgid, trainCount, trains, intersections);

    // execute responsibilities of server
    server_process(msgid, trainCount, intersectionCount, &trains, &intersections, RAG);

    // hard coded print statements to verify trains
    printf("\nTrain1's first intersection held: %s\n", trains[0].heldIntersections[0]);

    log_file = fopen("simulation.log", "a");
                   
    printSimulationComplete();
    
    fclose(log_file);

    createRAG_dot(trains, trainCount);
    RAG = createRAG_list(trains, trainCount);
    if (detectCycleInRAG(RAG)) {
        printf("\nWARNING: Cycle in RAG detected (deadlock can occur)\n");
    } else {
        printf("\nNo cycle in RAG detected\n");
    }

    printRAG_list(RAG);
    
    // clear up memory from message queue since, it is no longer needed
    destroyMessageQueue(msgid);

    FreeMemory(intersections, intersectionCount, trains, trainCount);

    // Clean up the dynamically allocated memory
    //free(intersections);
    freeRAG(RAG);
    //cleanupAll(trains, trainCount, intersections, intersectionCount);

    return 0;
}
