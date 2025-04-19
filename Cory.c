/*
Group D
Cory Thrutchley
cory.thrutchley@okstate.edu
4/19/2025
Description: This program contains the functions that are used for
handling process forking, child process (train) requests, parent 
process (server) responses, creation of resource allocation graphs,
and detection of cycles/deadlocks. 
*/

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

int releases = 0;

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
void server_process(int msgid, int trainCount, int intersectionCount, Train **trains, Intersection **intersections, Node *RAG) {
    Message msg;
    int routeLength = totalRouteLength(*trains, trainCount);

    while (releases < routeLength) {
        // Receive next message in the message queue from a train process
        msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0);

        // initialize current working train and intersection
        Train *train = &(*trains)[msg.trainIndex];  // Dereferencing trains pointer
        Intersection *targetIntersection = NULL;

        // identify address of current working intersection (targetIntersection)
        for (int i = 0; i < intersectionCount; i++) {
            if (strcmp((*intersections)[i].name, msg.intersectionName) == 0) {  // Dereferencing intersections pointer
                targetIntersection = &(*intersections)[i];  // Dereferencing intersections pointer
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

            // Intersection should be put in a state where it can be released again
            
            // %%%%%%%%% Keegan adding here: %%%%%%%%%%%%%%%%%%%%%%%%%
            if (targetIntersection && strcmp(targetIntersection->lock_type, "Mutex") == 0){
                // If the target intersection is mutex type
                // acquire the train's mutex and lock it
                // Keegan's function
                mutexAcqu(targetIntersection, train, msgid, msg.trainIndex, msg.intersectionName);
            } else if (targetIntersection && targetIntersection->lock_type && strcmp(targetIntersection->lock_type, "Semaphore") == 0) {
                // If the target intersection is semaphore type
                // acquire the train's semaphore 
                // Aiden's function
                semaphoreAcqu(targetIntersection, train, msgid, msg.trainIndex, msg.intersectionName);
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
                // keeping track of how many times an intersection has been release
                // so that the server will know once it's done (while loop conditional)
                releases++;

                printf("Releases%d\n", releases);
                // call for releasing mutexes and semaphores
                if(targetIntersection && strcmp(targetIntersection->lock_type, "Mutex") == 0){
                    printf("Mutex  %s releasing %s\n", targetIntersection->name, train->name);
                    pthread_mutex_unlock(&targetIntersection->Mutex);
                }
                if(targetIntersection && strcmp(targetIntersection->lock_type, "Semaphore") == 0){
                    printf("Semaphore %s releasing %s\n", targetIntersection->name, train->name);
                    sem_post(&targetIntersection->Semaphore);
                }
                if(targetIntersection != NULL){
                    // release intersection by removing it from train's held intersections list
                    free(train->heldIntersections[found]);
                }
                // Shift remaining intersections left in case a train has multiple intersections (wait, impossible??)
                for (int j = found; j < train->heldIntersectionCount - 1; j++) {
                    train->heldIntersections[j] = train->heldIntersections[j + 1];
                }
        
                // Null the last entry
                train->heldIntersections[train->heldIntersectionCount - 1] = NULL;
        
                // One less intersection
                train->heldIntersectionCount--;
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
        createRAG_dot(*trains, trainCount);  // Dereferencing trains
        RAG = createRAG_list(*trains, trainCount);  // Dereferencing trains
        if (detectCycleInRAG(RAG)) {
            printf("\nDeadlock detected!\n");
        
            // Declare inside the block
            #define MAX_DEADLOCKED_TRAINS 100
            #define NAME_LEN 50
        
            listTrainsInDeadlock(RAG);

            resolveDeadlock(trains, trainCount, intersections, intersectionCount, msgid); // Dereferencing trains and intersections
        }
    }

    // Wait for all trains/child processes to terminate
    for (int i = 0; i < trainCount; i++) {
        wait(NULL);
    }

    freeRAG(RAG);
}


// Function for train/child process behavior
void train_process(int msgid, int trainIndex, Train *trains, Intersection *intersections) {
    Message msg;
    Train *train = &trains[trainIndex];
    int travelTime;
    char *prevIntersection = NULL;
    srand(time(NULL));

    for (int i = 0; i < train->routeCount; i++) {
        // Set intersection to be the next one in the train's route
        char *intersectionName = train->route[i];
        // Randomize travel time
        travelTime = (rand() % 5) + 1;
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
                prevIntersection = NULL;
            }
            // Simulate travel time, random time from 1 to 8 seconds
            sleep(travelTime);
            // Update previous intersection for next iteration
            prevIntersection = intersectionName;
        } else if (msg.response == WAIT) {
            // %%%%% added this section and was not allowing deadlocks %%%%%
            // if (prevIntersection != NULL) {
            // trainRequest(RELEASE, msgid, trainIndex, prevIntersection);
            // prevIntersection = NULL;
            // }
            // Try again later
            printf("Server told Train%d to wait to acquire %s\n", trainIndex + 1, intersectionName);
            // wait a long time, then redo iteration to let the train try again
            sleep(6);
            if (i > 0) {
                i--;
            }
        } else if (msg.response == DENY) {
            // Request denied
            printf("Server denied Train%d to acquire %s\n", trainIndex + 1, intersectionName);
        }
    }

    // Release the last held intersection to complete the train's route
    if (prevIntersection != NULL) {
        trainRequest(RELEASE, msgid, trainIndex, prevIntersection);
        prevIntersection = NULL;
    }
    
    exit(0);
}

// Official function to fork trains
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
