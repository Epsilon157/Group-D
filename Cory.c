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
#include "shared_header.h"

#define MAX_TEXT 512

// Structure for message queue
struct message {
    long msg_type;      // Message type (used for routing)
    pid_t pid;          // Process ID of the sender
    char text[MAX_TEXT];// Request message
};

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

// Function for parent process acting as server
void server_process(int msgid, int trainCount) {
    struct message msg;

    // to do: will need to loop the server as long as
    // there are still trains that need to pass through

    for (int i = 0; i < trainCount; i++) {
        // Receive request from any child
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0) == -1) {
            perror("Parent process msgrcv failed");
            exit(1);
        }

        printf("Parent received: %s\n", msg.text);

        // Respond to the child
        msg.msg_type = msg.pid;  // Reply directly to the requesting child

        // ADD: will need to add checks to make sure there is room to 
        // allow the child processes to run (deadlock detection)
        snprintf(msg.text, MAX_TEXT, "Approved request for PID %d", msg.pid);

        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("Parent process msgsnd failed");
            exit(1);
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

void train_process(int msgid, int trainIndex) {
    struct message msg;
    msg.msg_type = 1; // Type 1 means request to the server
    msg.pid = getpid();
    snprintf(msg.text, MAX_TEXT, "Request from child PID %d", msg.pid);

    // to do: loop through each intersection in its route
    // requesting to go through them (one at a time?)

    // Send request to parent
    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("Child process msgsnd failed");
        exit(1);
    }

    // Receive response from the parent
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), msg.pid, 0) == -1) {
        perror("Child process msgrcv failed");
        exit(1);
    }

    // need a while loop to send requests for intersections
    // until all intersections in route are visited

    printf("Child %d received response: %s\n", getpid(), msg.text);
    exit(0);
}

// Temporary function for implementing forking,
// this is not official since this is what Fawaz
// will implement in Fawaz.c, except to match
// the required structure of trains
void fork_trains(int msgid, int trainCount) {
    for (int i = 0; i < trainCount; i++) {
        pid_t pid = fork();
        if (pid == 0) { // child processes run this
            train_process(msgid, i);
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
