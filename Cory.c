// Cory's Week 2 role: Basic IPC Workflow
// Crease a message queue system where different train processes can send messages to and receive messages from one another
// Testing: ensure that IPC successfully allows for message passing between trains

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_TEXT 512
#define NUM_CHILDREN 3

// Structure for message queue
struct message {
    long msg_type;      // Message type (used for routing)
    pid_t pid;          // Process ID of the sender
    char text[MAX_TEXT];// Request message
};

// Function for child process behavior
void child_process(int msgid) {
    struct message msg;
    msg.msg_type = 1; // Type 1 means request to the server
    msg.pid = getpid();
    snprintf(msg.text, MAX_TEXT, "Request from child PID %d", msg.pid);

    // Send request to parent
    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
        exit(1);
    }

    // Receive response from the parent
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), msg.pid, 0) == -1) {
        perror("msgrcv failed");
        exit(1);
    }

    printf("Child %d received response: %s\n", getpid(), msg.text);
    exit(0);
}

int main() {
    key_t key;
    int msgid;

    // Create a unique key for the message queue
    key = ftok("progfile", 65);
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

    // Fork multiple child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            child_process(msgid); // Child function
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
    }

    // Parent process acting as server
    struct message msg;
    for (int i = 0; i < NUM_CHILDREN; i++) {
        // Receive request from any child
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }

        printf("Parent received: %s\n", msg.text);

        // Respond to the child
        msg.msg_type = msg.pid;  // Reply directly to the requesting child
        snprintf(msg.text, MAX_TEXT, "Approved request for PID %d", msg.pid);

        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
    }

    // Wait for all children to terminate
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    // Cleanup: Remove message queue
    msgctl(msgid, IPC_RMID, NULL);

    printf("Parent: Message queue removed, exiting.\n");
    return 0;
}
