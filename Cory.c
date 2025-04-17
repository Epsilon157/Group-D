// Group D
// Cory Thrutchley
// cory.thrutchley@okstate.edu
// 4/15/2025

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "shared_header.h"
//#include "Keegan.c"

// Enumerations for train action and server response types
typedef enum {
    ACQUIRE,
    RELEASE
} TrainAction;

typedef enum {
    GRANT,
    WAIT,
    DENY
} ServerResponse;

// Structure for message queue
typedef struct msg_buffer {
    long msg_type;      // Message type (used for routing)
    int trainIndex;
    char intersectionName[50];
    int action;
    int response;
} Message;

// Graph data structure elements needed for resource
// allocation graph generation (digraph)
typedef struct Edge {
    char target[50];
    struct Edge *next;
} Edge;

typedef struct Node {
    char name[50];
    int isTrain;         // 1 = Train, 0 = Intersection
    Edge *edges;         // Outgoing edges
    struct Node *next;   // Next node in graph
} Node;

// Digraph helper function to find or create nodes
Node *findOrCreateNode(Node **head, const char *name, int isTrain) {
    Node *curr = *head;
    while (curr) {
        if (strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    }

    // Node not found: create and add
    Node *newNode = malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->isTrain = isTrain;
    newNode->edges = NULL;
    newNode->next = *head;
    *head = newNode;
    return newNode;
}

// Digraph helper function to add edges between 
// trains and instersections
void addEdge(Node *from, const char *toName) {
    Edge *newEdge = malloc(sizeof(Edge));
    strcpy(newEdge->target, toName);
    newEdge->next = from->edges;
    from->edges = newEdge;
}

// Digraph helper function to get a node by name 
// (used in graphDFS)
Node* getNodeByName(Node *graph, const char *name) {
    while (graph) {
        if (strcmp(graph->name, name) == 0)
            return graph;
        graph = graph->next;
    }
    return NULL;
}

// Digraph helper function used in depth-first
// search cycle detection algorithm
bool graphDFS(Node *node, Node *graph, bool *visited, bool *path, int index, char nodeNames[][50], int nodeCount) {
    visited[index] = true;
    path[index] = true;

    Edge *edge = node->edges;

    while (edge) {
        // Find the target node in the graph
        for (int i = 0; i < nodeCount; i++) {
            if (strcmp(edge->target, nodeNames[i]) == 0) {
                if (!visited[i]) {
                    if (graphDFS(getNodeByName(graph, nodeNames[i]), graph, visited, path, i, nodeNames, nodeCount))
                        return true;
                } else if (path[i]) {
                    // Back edge found: cycle detected
                    return true;
                }
            }
        }
        edge = edge->next;
    }

    path[index] = false;
    return false;
}

// Function for setting up the message queue
int createMessageQueue(int key, int msgid) {
    // Create a unique key for the message queue
    key = ftok("/tmp", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    // Create a message queue
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    return msgid;
}

// Function for clearing memory used by message
// queue at the end of the main function
void destroyMessageQueue(int msgid) {
    // Cleanup: Remove message queue
    msgctl(msgid, IPC_RMID, NULL);
    printf("Message queue removed\n");
}

// Creates a resource allocation graph in the form of
// a .dot file for visualization
void createRAG_dot(Train *trains, int trainCount) {
    FILE *fp = fopen("rag.dot", "w");
    if (!fp) {
        perror("Failed to open rag.dot");
        return;
    }

    fprintf(fp, "digraph RAG {\n");
    fprintf(fp, "  rankdir=LR;\n");
    fprintf(fp, "  node [shape=rectangle, style=filled, fillcolor=lightblue];\n");

    for (int i = 0; i < trainCount; i++) {
        Train *t = &trains[i];
        fprintf(fp, "  \"%s\" [shape=circle, fillcolor=lightgreen];\n", t->name);

        // Add edges from intersections to train (held)
        for (int j = 0; j < t->heldIntersectionCount; j++) {
            if (t->heldIntersections[j] && strlen(t->heldIntersections[j]) > 0) {
                fprintf(fp, "  \"%s\" -> \"%s\";\n", t->heldIntersections[j], t->name);
            }
        }

        // Add edge from train to intersection (waiting)
        if (t->waitingIntersection && strlen(t->waitingIntersection) > 0) {
            fprintf(fp, "  \"%s\" -> \"%s\";\n", t->name, t->waitingIntersection);
        }
    }

    fprintf(fp, "}\n");
    fclose(fp);

    printf("Resource Allocation Graph created: rag.dot\n");
}

// Creates a resource allocation graph in the form of
// a digraph to run the cycle detection function on
Node* createRAG_list(Train *trains, int trainCount) {
    Node *graph = NULL;

    for (int i = 0; i < trainCount; i++) {
        Train *t = &trains[i];
        Node *tNode = findOrCreateNode(&graph, t->name, 1);

        // Add edges from intersections to train (held)
        for (int j = 0; j < t->heldIntersectionCount; j++) {
            if (t->heldIntersections[j] && strlen(t->heldIntersections[j]) > 0) {
                Node *interNode = findOrCreateNode(&graph, t->heldIntersections[j], 0);
                addEdge(interNode, t->name);
            }
        }

        // Add edge from train to intersection (waiting)
        if (t->waitingIntersection && strlen(t->waitingIntersection) > 0) {
            Node *interNode = findOrCreateNode(&graph, t->waitingIntersection, 0);
            addEdge(tNode, t->waitingIntersection);
        }
    }

    return graph;
}

// Debugging function to print the RAG digraph to ensure
// RAG's are generated correctly
void printRAG_list(Node *graph) {
    Node *curr = graph;
    while (curr) {
        printf("%s -> ", curr->name);

        // different version of print statement to explicityly
        // state if the node is a train or intersection, not
        // necessary but helpful for debugging
        // printf("%s (%s) -> ", curr->name, curr->isTrain ? "Train" : "Intersection");

        Edge *e = curr->edges;
        while (e) {
            printf("%s ", e->target);
            e = e->next;
        }
        printf("\n");
        curr = curr->next;
    }
}

// Function to detect cycle in RAG
bool detectCycleInRAG(Node *graph) {
    // First, collect all node names
    int nodeCount = 0;
    Node *curr = graph;
    char nodeNames[100][50];  // Max 100 nodes
    while (curr && nodeCount < 100) {
        strcpy(nodeNames[nodeCount++], curr->name);
        curr = curr->next;
    }

    bool visited[100] = {false};
    bool path[100] = {false};

    for (int i = 0; i < nodeCount; i++) {
        if (!visited[i]) {
            Node *startNode = getNodeByName(graph, nodeNames[i]);
            if (graphDFS(startNode, graph, visited, path, i, nodeNames, nodeCount)) {
                return true; // Cycle detected
            }
        }
    }

    return false; // No cycles detected
}

void resolveDeadlock(Train *trains, int trainCount, Intersection *intersections, int intersectionCount, int msgid) {
    // Simple strategy: pick the train holding the most resources
    int victimIndex = -1;
    int maxHeld = 0;

    for (int i = 0; i < trainCount; i++) {
        if (trains[i].heldIntersectionCount > maxHeld) {
            maxHeld = trains[i].heldIntersectionCount;
            victimIndex = i;
        }
    }

    if (victimIndex == -1) return;

    Train *victim = &trains[victimIndex];
    printf("Preempting Train%d (%s)\n", victimIndex + 1, victim->name);
    
    for (int i = 0; i < victim->heldIntersectionCount; i++) {
        char *intersectionName = victim->heldIntersections[i];
        Intersection *targetIntersection = NULL;
        
        for (int j = 0; j < intersectionCount; j++) {
            if (strcmp(intersections[j].name, intersectionName) == 0) {
                targetIntersection = &intersections[j];
                break;
            }
        }

        if (targetIntersection != NULL) {
            if (strcmp(targetIntersection->lock_type, "Mutex") == 0) {
                releaseTrainMutex(targetIntersection, victim->name);
            } else if (strcmp(targetIntersection->lock_type, "Semaphore") == 0) {
                releaseTrain(targetIntersection, victim->name);
            }
        }

        // Logging
        log_file = fopen("simulation.log", "a");
        AttemptingDeadlockResolve(intersectionName, victim->name);
        fclose(log_file);

        free(intersectionName);
        victim->heldIntersections[i] = NULL;
    }

    victim->heldIntersectionCount = 0;
}

// Function for a train to request to ACQUIRE or RELEASE an intersection
void trainRequest(TrainAction act, int msgid, int trainIndex, const char *intersectionName) {
    Message msg;
    msg.msg_type = 1; // Type 1 = train-to-server
    msg.trainIndex = trainIndex;
    strcpy(msg.intersectionName, intersectionName);
    msg.action = act;
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
}

// Function for server to GRANT, WAIT, or DENY a train's request
void serverResponse(ServerResponse resp, int msgid, int trainIndex, const char *intersectionName) {
    Message msg;
    msg.msg_type = trainIndex + 100; // Each train listens on its own msg_type
    strcpy(msg.intersectionName, intersectionName);
    msg.response = resp;
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
}

// Function to determine how many times a train will
// have to go through an intersection, used for
// server_process function to know how long to loop
int totalRouteLength(Train *trains, int trainCount) {
    int count = 0;

    for (int i = 0; i < trainCount; i++) {
        // printf("Route count: %d\n", trains[i].routeCount);
        count = count + trains[i].routeCount;
    }

    return count;
}

// Function for parent process acting as server
void server_process(int msgid, int trainCount, int intersectionCount, Train *trains, Intersection *intersections, Node *RAG) {
    Message msg;
    int releases = 0;
    int routeLength = totalRouteLength(trains, trainCount);

    while (releases < routeLength) {
        // Receive next message in the message queue from a train process
        msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0);

        // initialize current working train and intersection
        Train *train = &trains[msg.trainIndex];
        Intersection *targetIntersection = NULL;

        // identify address of current working intersection (targetIntersection)
        for (int i =0; i < intersectionCount; i++) {
            if (strcmp(intersections[i].name, msg.intersectionName) == 0) {
                targetIntersection = &intersections[i];
                break;
            }
        }

        // When a train wants to acquire an intersection
        if (msg.action == ACQUIRE) {
            // logging
            log_file = fopen("simulation.log", "a");
            printRequestSent(msg.trainIndex, msg.intersectionName);
            fclose(log_file);

            // server recognizes acquire request
            printf("Train%d request to acquire %s\n", msg.trainIndex + 1, msg.intersectionName);
            
            // %%%%%%%%% Keegan adding here: %%%%%%%%%%%%%%%%%%%%%%%%%
            // Keegan added a semaphore section calling Aidens acquireTarin function
            // teamwork! :D

            if (targetIntersection && strcmp(targetIntersection->lock_type, "Mutex") == 0){
                // If the target intersection is mutex type

                // acquire the train's mutex and lock it
                if(targetIntersection-> lock_state == 0){
                acquireTrainMutex(targetIntersection, trains[msg.trainIndex].name);
                printf("Server attempting tryAcquireMutex for Train %s on %s\n", train->name, msg.intersectionName);

                // grant the request to the train
                serverResponse(GRANT, msgid, msg.trainIndex, msg.intersectionName);

                // logging
                log_file = fopen("simulation.log", "a");
                printIntersectionGranted(msg.trainIndex, msg.intersectionName);
                fclose(log_file);

                // update train to be holding the intersection
                train->heldIntersections[train->heldIntersectionCount] = strdup(msg.intersectionName); // safe string copy
                train->heldIntersectionCount++; 

                // train is no longer waiting on this intersection, so remove it from the waiting list
                free(train->waitingIntersection);
                train->waitingIntersection = NULL; // Clear it after successful grant
                }
                else {
                    // It's locked, deny for now
                    printf("Train%d cannot acquire %s, already locked. Sending WAIT.\n", msg.trainIndex + 1, msg.intersectionName);
                    serverResponse(WAIT, msgid, msg.trainIndex, msg.intersectionName);
            
                    free(train->waitingIntersection);
                    train->waitingIntersection = strdup(msg.intersectionName);
            
                    
                }
            } else if (targetIntersection && targetIntersection->lock_type && strcmp(targetIntersection->lock_type, "Semaphore") == 0) {
                // If the target intersection is semaphore type

                // acquire the train's semaphore
                if(strcmp(targetIntersection->lock_type, "Semaphore") ==0){
                acquireTrain(targetIntersection, trains[msg.trainIndex].name);

                // grant the request to the train
                serverResponse(GRANT, msgid, msg.trainIndex, msg.intersectionName);
                log_file = fopen("simulation.log", "a");
                printIntersectionGranted(msg.trainIndex, msg.intersectionName);
                fclose(log_file);
                // update train to be holding the intersection
                train->heldIntersections[train->heldIntersectionCount] = strdup(msg.intersectionName); // safe string copy
                train->heldIntersectionCount++;

                // train is no longer waiting on this intersection, so remove it from the waiting list
                free(train->waitingIntersection);
                train->waitingIntersection = NULL; // Clear it after successful grant
                }
            } else {
                // If the train can't obtain a mutex or semaphore (if the intersection is full)
                printf("Train%d can't obtain %s, sending WAIT\n", msg.trainIndex + 1, msg.intersectionName);
                serverResponse(WAIT, msgid, msg.trainIndex, msg.intersectionName);

                // Add the intersection to the train's waiting list
                free(train->waitingIntersection); // Avoid memory leak before overwriting
                train->waitingIntersection = strdup(msg.intersectionName);
            } 
        } else if (msg.action == RELEASE) {
            // logging
            log_file = fopen("simulation.log", "a");
            printRequestRelease(msg.trainIndex, msg.intersectionName);
            fclose(log_file);

            // server recognizes release request
            printf("Train%d request to release %s\n", msg.trainIndex + 1, msg.intersectionName);

            // variable used to find index of intersection to remove from train
            int found = -1;
            
            // Loop to find index of intersection to remove
            for (int i = 0; i < train->heldIntersectionCount; i++) {
                if (strcmp(train->heldIntersections[i], msg.intersectionName) == 0) {
                    found = i;
                    break;
                }
            }
            
            // Safely remove the intersection from the train list
            if (found != -1) {
                // call for releasing mutexes and semaphores
                if(targetIntersection != NULL){
                    if(strcmp(targetIntersection -> lock_type, "Mutex")== 0){
                        releaseTrainMutex(targetIntersection, train->name);
                    }  
                    else if(strcmp(targetIntersection -> lock_type, "Semaphore")== 0){
                        releaseTrain(targetIntersection, train->name);
                    }
                }
                // release intersection by removing it from train's held intersections list
                free(train->heldIntersections[found]);
        
                // Shift remaining intersections left in case a train has multiple intersections (wait, impossible??)
                for (int j = found; j < train->heldIntersectionCount - 1; j++) {
                    train->heldIntersections[j] = train->heldIntersections[j + 1];
                }
        
                // Null the last entry
                train->heldIntersections[train->heldIntersectionCount - 1] = NULL;
        
                // One less intersection
                train->heldIntersectionCount--;

                // keeping track of how many times an intersection has been release
                // so that the server will know once it's done (while loop conditional)
                releases++;
            } else {
                printf("WARNING: Train%d tried to release an intersection it does not hold: %s\n", msg.trainIndex + 1, msg.intersectionName);
            }
        }

        // reset the clock for the simulation log
        pthread_mutex_lock(&sim_time_mutex);
        (*sim_time)++;  // One increment per simulation loop
        log_file = fopen("simulation.log", "a");
        fprintf(log_file, "\n");
        fclose(log_file);
        pthread_mutex_unlock(&sim_time_mutex);

        // Graph the current state of the program (RAG)
        createRAG_dot(trains, trainCount);
        RAG = createRAG_list(trains, trainCount);
        if (detectCycleInRAG(RAG)) {
            printf("\nDeadlock detected!\n");
        
            // Declare inside the block
            #define MAX_DEADLOCKED_TRAINS 100
            #define NAME_LEN 50
        
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
            resolveDeadlock(trains, trainCount, intersections, intersectionCount, msgid);
        } else {
            // printf("\nNo cycle in RAG detected\n");
        }
    }

    // Wait for all trains/child processes to terminate
    for (int i = 0; i < trainCount; i++) {
        wait(NULL);
    }
}



// Function for train/child process behavior
void train_process(int msgid, int trainIndex, Train *trains, Intersection *intersections) {
    Message msg;
    Train *train = &trains[trainIndex];
    int travelTime;
    char *prevIntersection = NULL;

    for (int i = 0; i < train->routeCount; i++) {
        // Set intersection to be the next one in the train's route
        char *intersectionName = train->route[i];

        // Request to acquire an intersection
        trainRequest(ACQUIRE, msgid, trainIndex, intersectionName);

        // Wait for server response to acquire request
        msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), trainIndex + 100, 0);

        if (msg.response == GRANT) {
            // Request granted
            printf("Server granted Train%d to acquire%s\n", trainIndex + 1, intersectionName);
            // Release previous intersection after acquiring and travelling
            if (prevIntersection != NULL) {
                trainRequest(RELEASE, msgid, trainIndex, prevIntersection);
            }
            // Simulate travel time, random time from 1 to 8 seconds
            srand(time(NULL));
            travelTime = (rand() % 8) + 1;
            sleep(travelTime);
            // Update previous intersection for next iteration
            prevIntersection = intersectionName;
        } else if (msg.response == WAIT) {

// %%%%% added this section and was not allowing deadlocks %%%%%
           // if (prevIntersection != NULL) {
             //   trainRequest(RELEASE, msgid, trainIndex, prevIntersection);
              //  prevIntersection = NULL;
            //}
            // Try again later
            printf("Server told Train%d to wait to acquire %s\n", trainIndex + 1, intersectionName);
            // wait a long time, then redo iteration to let the train try again
            sleep(10);
            i--;
        } else if (msg.response == DENY) {
            // Request denied
            // to do: figure out when to deny a request instead of telling it to wait
            printf("Server denied Train%d to acquire %s\n", trainIndex + 1, intersectionName);
        }
    }

    // Release the last held intersection to complete the train's route
    if (prevIntersection != NULL) {
        trainRequest(RELEASE, msgid, trainIndex, prevIntersection);
    }
    
    exit(0);
}

// Temporary function for implementing forking,
// this is not official since this is what Fawaz
// will implement in Fawaz.c, except to match
// the required structure of trains
void fork_trains(int msgid, int trainCount, Train *trains, Intersection *intersections) {
    for (int i = 0; i < trainCount; i++) {
        pid_t pid = fork();
        if (pid == 0) { // child processes run this
            train_process(msgid, i, trains, intersections);
        } else if (pid < 0) { // only runs if a fork fails
            perror("fork failed");
            exit(1);
        }
    }
}
