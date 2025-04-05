// Group D
// Cory Thrutchley
// cory.thrutchley@okstate.edu

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

#define MAX_TEXT 512
#define NUM_CHILDREN 3

// Structure for message queue
struct message {
    long msg_type;      // Message type (used for routing)
    pid_t pid;          // Process ID of the sender
    char text[MAX_TEXT];// Request message
};

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
    printf("Message queue removed, exiting.\n");
}

// Function for parent process acting as server
void parent_process(int msgid) {
    struct message msg;
    // CHANGE: will need to loop the server as long as
    // there are still trains that need to pass through
    for (int i = 0; i < NUM_CHILDREN; i++) {
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
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }
}

// Function for child process behavior
// This is temporary for testing message queue system, a function 
// specific to train requirements should replace this
void child_process(int msgid) {
    struct message msg;
    msg.msg_type = 1; // Type 1 means request to the server
    msg.pid = getpid();
    snprintf(msg.text, MAX_TEXT, "Request from child PID %d", msg.pid);

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

    printf("Child %d received response: %s\n", getpid(), msg.text);
    exit(0);
}

// Temporary function for implementing forking,
// this is not official since this is what Fawaz
// will implement in Fawaz.c, except to match
// the required structure of trains
void fork_child_processes(int msgid) {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == 0) { // child processes run this
            child_process(msgid);
        } else if (pid < 0) { // only runs if a fork fails
            perror("fork failed");
            exit(1);
        }
    }
}

// This main function will need to be removed, this is only for
// forking and IPC testing purposes
int main() {
    // create key and message queue ID needed for the
    // message queue to work
    key_t key;
    int msgid;

    // Create message queue for message passing between parent
    // and child processes. This function MUST return msgid so 
    // that the created message queue ID can be used in the 
    // rest of the main function.
    msgid = createMessageQueue(key, msgid);

    // Fork multiple child processes
    // this is purely for testing
    fork_child_processes(msgid);

    // By this point, all child processes have exited,
    // so only the parent will execute the code below

    // execute responsibilities of server
    parent_process(msgid);

    // clear up memory from message queue since, it is no longer needed
    destroyMessageQueue(msgid);

    return 0;
}
