
// Description: Event-driven logging implementation for Week 3 - Simulation Logging.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// Shared memory structure for simulation time
typedef struct {
    int sim_time;
    pthread_mutex_t time_mutex;
} sim_time_t;

static sim_time_t *shared_time = NULL;
static FILE *log_file = NULL;

// Initialize shared logging resources (call this once in parent process)
int init_simulation_logging() {
    // Create shared memory for timekeeping
    int shm_id = shmget(IPC_PRIVATE, sizeof(sim_time_t), IPC_CREAT | 0666);
    if (shm_id == -1) return -1;

    shared_time = (sim_time_t *)shmat(shm_id, NULL, 0);
    if (shared_time == (void *)-1) return -1;

    shared_time->sim_time = 0;
    pthread_mutex_init(&shared_time->time_mutex, NULL);

    // Open log file once (server process only)
    log_file = fopen("simulation.log", "a");
    if (!log_file) return -1;

    return 0;
}

// Format the sim_time into [HH:MM:SS] format
static void format_time(char* buffer) {
    int hours = shared_time->sim_time / 3600;
    int minutes = (shared_time->sim_time % 3600) / 60;
    int seconds = shared_time->sim_time % 60;
    sprintf(buffer, "[%02d:%02d:%02d]", hours, minutes, seconds);
}

// Thread-safe logging function
void log_event(const char* format, ...) {
    if (!shared_time || !log_file) return;

    char timestamp[12];
    
    pthread_mutex_lock(&shared_time->time_mutex);
    // Increment time and format
    shared_time->sim_time++;
    format_time(timestamp);
    pthread_mutex_unlock(&shared_time->time_mutex);

    // Write to log
    pthread_mutex_lock(&shared_time->time_mutex);
    fprintf(log_file, "%s ", timestamp);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file); // Ensure immediate write
    pthread_mutex_unlock(&shared_time->time_mutex);
}

// Cleanup function (call before exit)
void cleanup_logging() {
    if (log_file) fclose(log_file);
    if (shared_time) {
        pthread_mutex_destroy(&shared_time->time_mutex);
        shmdt(shared_time);
    }
}

// Convenience functions for specific log types
void log_server_init() {
    log_event("SERVER: Initialized intersections:");
    // Add intersection details here...
}

void log_train_request(const char* train, const char* intersection) {
    log_event("TRAIN%s: Sent ACQUIRE request for Intersection%s.", train, intersection);
}

void log_grant(const char* train, const char* intersection, int capacity) {
    if (capacity > 1) {
        log_event("SERVER: GRANTED Intersection%s to Train%s. Semaphore count: %d", 
                 intersection, train, capacity-1);
    } else {
        log_event("SERVER: GRANTED Intersection%s to Train%s", intersection, train);
    }
}