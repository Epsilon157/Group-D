// Group D
// Cory Thrutchley
// cory.thrutchley@okstate.edu
// 4/5/2025

// Cory's Week 2 role: Basic IPC Workflow
// Crease a message queue system where different train processes 
// can send messages to and receive messages from one another

// Testing: ensure that IPC successfully allows for message 
// passing between trains

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

// Digraph helper function to find/create nodes
Node *findOrCreateNode(Node **head, const char *name, int isTrain) {
    Node *curr = *head;
    while (curr) {
        if (strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    }

    // Not found — create and prepend
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

// function to determine how many times a train will
// have to go through an intersection, only used for
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
void server_process(int msgid, int trainCount, Train *trains, Intersection *intersections) {
    // to do: will need to loop the server as long as
    // there are still trains that need to pass through

    Message msg;
    int grants = 0;
    int routeLength = totalRouteLength(trains, trainCount);

    // printf("Total route length is %d\n", routeLength);

    // This currently loops as many times as there are 
    // trains * intersections, this is temporary.
    // We will need a much better way of telling when every
    // train has passed through.
    while (grants < routeLength) {
        msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0);

        Train *train = &trains[msg.trainIndex];

        if (msg.action == ACQUIRE) {
            // server recognizes acquire request
            printf("Train%d request to acquire %s\n", msg.trainIndex + 1, msg.intersectionName);

            // to do: add conditionals to tell whether the intersection the train is
            // requesting for is in use or not. If the intersection is in use, then
            // tell the train to wait using serverResponse(WAIT). If the intersection 
            // is free, then grant the train's request using serverResponse(GRANT)

            // to do: add a way to update/access intersection.lock_state
            // to decide whether to grant, wait, or deny

            serverResponse(GRANT, msgid, msg.trainIndex, msg.intersectionName);
            train->heldIntersections[train->heldIntersectionCount] = strdup(msg.intersectionName); // safe string copy
            train->heldIntersectionCount++;
            grants++;

        } else if (msg.action == RELEASE) {
            // server recognizes release request
            printf("Train%d request to release %s\n", msg.trainIndex + 1, msg.intersectionName);
            serverResponse(GRANT, msgid, msg.trainIndex, msg.intersectionName);
            // to do: find a good way to have each train release intersections after travelling through them.
            // This probably can't be done in train_process after sleep(2) because trains don't have access to main memory
        }
    }

    // Wait for all children to terminate
    for (int i = 0; i < trainCount; i++) {
        wait(NULL);
    }
}

// Function for child process behavior
// This is temporary for testing message queue system, a function 
// specific to train requirements should replace this

// , Train *trains, Intersection *intersections

void train_process(int msgid, int trainIndex, Train *trains, Intersection *intersections) {
    // to do: loop through each intersection in its route
    // requesting to go through them (one at a time?)

    Message msg;
    Train *train = &trains[trainIndex];

    for (int i = 0; i < train->routeCount; i++) {
        char *intersectionName = train->route[i];

        trainRequest(ACQUIRE, msgid, trainIndex, intersectionName);

        msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), trainIndex + 100, 0);

        if (msg.response == GRANT) {
            // request granted
            printf("Train%d granted %s\n", trainIndex + 1, intersectionName);
            // simulate travel time
            sleep(2);
        } else if (msg.response == WAIT) {
            // try again later
            printf("Train%d must wait for %s\n", trainIndex + 1, intersectionName);
            // wait a long time, then redo iteration to let the train try again
            sleep(10);
            i--;
        } else if (msg.response == DENY) {
            // request denied
            // Question: when to deny a request instead of telling it to wait?
            printf("Train%d denied access to %s\n", trainIndex + 1, intersectionName);
        }
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

// // This main function will need to be removed, this is only for
// // forking and IPC testing purposes
// int main() {
//     // create key and message queue ID needed for the
//     // message queue to work
//     key_t key;
//     int msgid;

//     // testing 
//     int trainCount = 3;

//     // Create message queue for message passing between parent
//     // and child processes. This function MUST return msgid so 
//     // that the created message queue ID can be used in the 
//     // rest of the main function.
//     msgid = createMessageQueue(key, msgid);

//     // Fork multiple child processes
//     // this is purely for testing
//     fork_trains(msgid, trainCount);

//     // By this point, all child processes have exited,
//     // so only the parent will execute the code below

//     // execute responsibilities of server
//     server_process(msgid, trainCount);

//     // clear up memory from message queue since, it is no longer needed
//     destroyMessageQueue(msgid);

//     return 0;
// }
