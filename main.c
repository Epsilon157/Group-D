//Main, combined file
/* Add text options when ran so anyone can find how to pull the data out.
 As we get more code for the data to be used in I will adjust parsing so it returns the 
 info needed*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "shared_header.h"

// include other files here

#include "Cory.c"
#include "Keegan.c" 
#include "Aiden.c"
#include "Jacob.c"
// #include "Fawaz.c"
#include "Zack.c"

/*// Structure to hold data about trains
typedef struct {
    char name[50];  // Train name (e.g., Train1)
    char **route;   // Array of intersection names the train passes through
    int routeCount; // Number of intersections the train visits
} Train;

// Structure to hold data about intersections
typedef struct {
    char name[50];  // Intersection name
	char type[50]; // Type of Intersection
	char lock_type[50]; //Type of lock
    int resources;  // Resources available at this intersection
	int lock_state; //0 is for free, 1 is for locked
	Train trains[50];
	
	pthread_mutex_t Mutex;
	sem_t Semaphore;
	
} Intersection;*

// File paths for intersections and trains
const char *intersectionFilePath = "intersections.txt";
const char *trainFilePath = "trains.txt";
*/

/*
//Function that makes the resource allocation table
void initR_Table(Intersection **intersections, int intersectionCount){
	
	for(int i = 0; i < intersectionCount; i++){
		
		//initializes lock_state to 0
		(*intersections)[i].lock_state = 0;
		
		//Test case
		strcpy((*intersections)[0].trains[0].name, "Train 1");
		strcpy((*intersections)[1].trains[0].name, "Train 2");
		strcpy((*intersections)[1].trains[1].name, "Train 3");
		
		//initializes semaphore is resource > 1 and mutex if else
		if((*intersections)[i].resources > 1){
			strcpy((*intersections)[i].lock_type, "Semaphore");
		} else{
			strcpy((*intersections)[i].lock_type, "Mutex");
		}
		
		if(strcmp((*intersections)[i].lock_type, "Semaphore")){
			sem_init(&(*intersections)[i].Semaphore, 0, (*intersections)[i].resources);
		} else{
			pthread_mutex_init(&(*intersections)[i].Mutex, NULL);
		}
		
		
	} 

    //initializeMutex(*intersections, intersectionCount);
	
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
               (*intersections)[i].resources, ((*intersections)[i].lock_state == 0) ? "Free" : "Locked");
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
*/


/*
// Function to parse the intersection data
int IntersectionParsing(const char *filename, Intersection **intersections) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    rewind(file);

    *intersections = malloc(count * sizeof(Intersection));

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char name[50];
        int resources;
        sscanf(line, "%[^:]:%d", name, &resources);
        strcpy((*intersections)[i].name, name);
        (*intersections)[i].resources = resources;
        i++;
    }

    fclose(file);
    return count;
}

// Function to parse the train data
int TrainParsing(const char *filename, Train **trains) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    int count = 0;
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    rewind(file);

    *trains = malloc(count * sizeof(Train));

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char name[50];
        char route[200];
        sscanf(line, "%[^:]:%s", name, route);

        strcpy((*trains)[i].name, name);

        (*trains)[i].route = NULL;
        (*trains)[i].routeCount = 0;

        char *token = strtok(route, ",");
        while (token != NULL) {
            (*trains)[i].route = realloc((*trains)[i].route, ((*trains)[i].routeCount + 1) * sizeof(char *));
            (*trains)[i].route[(*trains)[i].routeCount] = malloc(strlen(token) + 1);
            strcpy((*trains)[i].route[(*trains)[i].routeCount], token);
            (*trains)[i].routeCount++;
            token = strtok(NULL, ",");
        }
        i++;
    }

    fclose(file);
    return count;
}

// Function to get the resource amount for a specific intersection
void GetIntersectionResources(Intersection *intersections, int intersectionCount, int intersectionIndex) {
    if (intersectionIndex >= 1 && intersectionIndex <= intersectionCount) {
        printf("Resources at intersection %s: %d\n", intersections[intersectionIndex - 1].name, intersections[intersectionIndex - 1].resources);
    } else {
        printf("Invalid intersection number.\n");
    }
}*/

int main() {
    Intersection *intersections; //From * to **
    Train *trains;
    // create key and message queue ID needed for the
    // message queue to work
    key_t key;
    int msgid;
    //below, should it be & or * or **?
    int intersectionCount = IntersectionParsing(intersectionFilePath, &intersections);
    printf("Parsed %d intersections:\n", intersectionCount);
    for (int i = 0; i < intersectionCount; i++) {
        printf("%s has %d resources\n", intersections[i].name, intersections[i].capacity);
    }

    // removed &
    initializeSemaphores(intersections, intersectionCount);//added
    initializeMutex(intersections, intersectionCount);//added


    int trainCount = TrainParsing(trainFilePath, &trains);
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

    initR_Table(&intersections, intersectionCount);
	printR_Table(&intersections, intersectionCount);


    // This is just testing mutexes are properly working
    printf("\n TEST FOR MUTEX \n") ;
    test_initializeMutex();
    printf("\n END OF TEST FOR MUTEX \n") ;

	/*  // Main menu for user to choose
    int choice;
    printf("\nSelect an option:\n");
    printf("1. Select an intersection and find all trains passing through it\n");
    printf("2. Select a stop number (1-%d) and find all trains with that stop\n", trainCount);
    printf("3. Get the path of a specific train\n");
    printf("4. Select an intersection and get the resource amount\n");
    printf("Enter your choice (1, 2, 3, or 4): ");
    scanf("%d", &choice);

    if (choice == 1) {
        int stopNumber;
        printf("Enter the intersection number (1-%d): ", intersectionCount);
        scanf("%d", &stopNumber);

        if (stopNumber >= 1 && stopNumber <= intersectionCount) {
            const char *intersectionName = intersections[stopNumber - 1].name;
            printf("Trains passing through %s:\n", intersectionName);
            for (int i = 0; i < trainCount; i++) {
                for (int j = 0; j < trains[i].routeCount; j++) {
                    if (strcmp(trains[i].route[j], intersectionName) == 0) {
                        printf("%s\n", trains[i].name);
                        break;
                    }
                }
            }
        } else {
            printf("Invalid intersection number.\n");
        }
    } else if (choice == 2) {
        int stopNumber;
        printf("Enter the stop number (1-%d): ", trainCount);
        scanf("%d", &stopNumber);

        printf("Trains at stop %d:\n", stopNumber);
        for (int i = 0; i < trainCount; i++) {
            if (stopNumber <= trains[i].routeCount) {
                printf("%s: %s (Stop %d)\n", trains[i].name, trains[i].route[stopNumber - 1], stopNumber);
            }
        }
    } else if (choice == 3) {
        char trainName[50];
        printf("Enter the train name: ");
        scanf("%s", trainName);
        DisplayTrainPath(trains, trainCount, trainName);
    } else if (choice == 4) {
        int intersectionNumber;
        printf("Enter the intersection number (1-%d): ", intersectionCount);
        scanf("%d", &intersectionNumber);

        GetIntersectionResources(intersections, intersectionCount, intersectionNumber);
    } else {
        printf("Invalid choice.\n");
    }
	*/

    // Create message queue for message passing between parent
    // and child processes. This function MUST return msgid so 
    // that the created message queue ID can be used in the 
    // rest of the main function.
    msgid = createMessageQueue(key, msgid);

    // Fork multiple child processes
    // this is purely for testing
    fork_trains(msgid, trainCount);

    // By this point, all child processes have exited,
    // so only the parent will execute the code below

    // execute responsibilities of server
    server_process(msgid, trainCount);

    // running createRAG in server for now for testing, it should
    // eventually be executed after any train request is made
    // in the train_process function

    // for this to work properly, trains.heldIntersections and
    // trains.instersectionCount need to be updated based on
    // parsed info from text files
    
    // hard coded tests for RAG
    trains[0].heldIntersectionCount = 2;
    trains[0].heldIntersections[0] = "IntersectionA";
    trains[0].heldIntersections[1] = "IntersectionB";
    trains[0].waitingIntersection = "IntersectionC";

    trains[1].heldIntersectionCount = 1;
    trains[1].heldIntersections[0] = "IntersectionD";

    trains[2].heldIntersectionCount = 0;
    trains[2].waitingIntersection = "IntersectionD";
    
    trains[3].heldIntersectionCount = 1;
    trains[3].heldIntersections[0] = "IntersectionC";
    trains[3].waitingIntersection = "IntersectionA";

    createRAG_dot(trains, trainCount);
    Node *RAG = createRAG_list(trains, trainCount);
    printRAG_list(RAG);

    if (detectCycleInRAG(RAG)) {
        printf("\nCycle in RAG detected (deadlock can occur)\n");
    } else {
        printf("\nNo cycle in RAG detected\n");
    }

    // clear up memory from message queue since, it is no longer needed
    destroyMessageQueue(msgid);

    FreeMemory(intersections, intersectionCount, trains, trainCount);

    int num_intersections = IntersectionParsing("intersections.txt", &intersections);
    if (num_intersections > 0) {
        // Print the initialized intersections
        print_initialized_intersections(intersections, num_intersections);
    } else {
        printf("No intersections to display.\n");
    }

    // Clean up the dynamically allocated memory
    free(intersections);

    log_file = fopen("simulation.log", "w");
    if (log_file == NULL) {
        perror("Failed to open simulation.log");
        exit(EXIT_FAILURE);
    }

     
    print_initialized_intersections(intersections, 5);

    if (log_file) {
        fclose(log_file);
    }




    
    return 0;
}
