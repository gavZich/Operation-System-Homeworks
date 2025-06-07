#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>


#define MAX_PROCESSES 1000
#define MAX_LINE 256
#define MAX_NAME 50
#define MAX_DESCRIPTION 100

// define enum of sort vertions to modify the sort to the relevent algorithem
typedef enum {
    SORT_BY_ARRIVAL,
    SORT_BY_BURST,
    SORT_BY_PRIORITY
} SortMode;
// choose which key to sort by before calling qsort()
static SortMode sortMode;

// Global variables
int processCount = 0;

typedef struct process {
    pid_t pid; 
    char name[MAX_NAME];
    char description[MAX_DESCRIPTION];
    int arrivalTime;
    int burstTime;
    int priority;
    int waitingTime;
    int started;
    int isFinished;       // Flag: 1 if process has already exited
    int remainingTime;
    int originalIndex;
} Process;

Process processes[MAX_PROCESSES];

void parseProcessesFromFile(const char* filePath) {
    FILE* file = fopen(filePath, "r");
       if (file == NULL) {
        perror("Error opening file");
        return;
    }
    
    char buffer[MAX_LINE];
    processCount = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && processCount < MAX_PROCESSES) {
        Process* p = &processes[processCount];
        buffer[strcspn(buffer, "\r\n")] = '\0';

        char *token = strtok(buffer, ",");
        // empty or malformed line—skip it
        if (token == NULL) {
            continue;
        }
        strncpy(processes[processCount].name, token, MAX_NAME - 1);
        p->name[MAX_NAME - 1] = '\0';

        token = strtok(NULL, ",");
        strncpy(processes[processCount].description, token, MAX_DESCRIPTION - 1);
        p->description[MAX_DESCRIPTION - 1] = '\0';


        token = strtok(NULL, ",");
        p->arrivalTime = atoi(token);

        
        token = strtok(NULL, ",");
        p->burstTime = atoi(token);

        
        token = strtok(NULL, ",");
        p->priority = atoi(token); 

        p->originalIndex = processCount;

        processCount++;
        
    }

    fclose(file);
}

// find Process* by pid
Process* find_process_by_pid(pid_t p) {
    for (int i = 0; i < processCount; i++) {
        if (processes[i].pid == p) {
            return &processes[i];
        }
    }
    return NULL;
}

// start an alarm by the remainig time of the process
// and pause it untill we get SIGALRM
void handle_sigcont(int signum) {
    Process *process = find_process_by_pid(getpid());
      if (process != NULL) {
        alarm(process->remainingTime);
    }
    // puse untill SIGALRM arrive
    pause();
    
}

// SIGALRM handler in the child process:
// When SIGALRM arrives, while burst time is over, the child simply exits
void handle_sigalrm(int signum) {
    exit(0);
}

// Function executed by each child process after fork()
void child_main(Process* process) {
    // Initialize the burst time for this child
    int childBurstTime = process->burstTime;

    // Set up SIGCONT handler
    struct sigaction sa_cont;
    sa_cont.sa_handler = handle_sigcont;
    sigemptyset(&sa_cont.sa_mask);
    sa_cont.sa_flags = 0;
    sigaction(SIGCONT, &sa_cont, NULL);

    // Set up SIGALRM handler
    struct sigaction sa_alrm;
    sa_alrm.sa_handler = handle_sigalrm;
    sigemptyset(&sa_alrm.sa_mask);
    sa_alrm.sa_flags = 0;
    sigaction(SIGALRM, &sa_alrm, NULL);

    // Immediately stop itself until the scheduler sends SIGCONT
    raise(SIGSTOP);

    // After receiving SIGCONT, pause until SIGALRM signals burst completion
    pause();
}


// generic comparator: primary key = sortMode, 
// secondary key = arrivalTime (unless already used), 
// final tiebreak = originalIndex
int cmp_process(const void *a, const void *b) {
    const Process *pa = a, *pb = b;
    int diff;

    // primary key
    switch (sortMode) {
        case SORT_BY_ARRIVAL:
            diff = pa->arrivalTime - pb->arrivalTime;
            break;
        case SORT_BY_BURST:
            diff = pa->burstTime - pb->burstTime;
            break;
        case SORT_BY_PRIORITY:
            diff = pa->priority - pb->priority;
            break;
    }
    if (diff) {
        return diff;
    } 
    
    // secondary key: arrivalTime if primary≠arrival
    if (sortMode != SORT_BY_ARRIVAL) {
        diff = pa->arrivalTime - pb->arrivalTime;
        if (diff) return diff;
    }

    // final tiebreak: preserve input order
    return pa->originalIndex - pb->originalIndex;
}

// sortMode = SORT_BY_BURST;
// qsort(processes, processCount, sizeof *processes, cmp_process);

// function that clculate an avrage
int avrage(int totalWatingTime, int n) {
    return totalWatingTime / n;
}

// FCFS algorithem
void run_fcfs(Process *processes, int processCount) {
    // Header
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : FCFS\n");
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");

    // sort by arrival time (stable)
    sortMode = SORT_BY_ARRIVAL;
    qsort(processes, processCount, sizeof(Process), cmp_process);

    int currentTime = 0;
    int totalWaitingTime = 0;

    for (int i = 0; i < processCount; i++) {
        Process *p = &processes[i];

        // idle if next arrival is in the future
        if (p->arrivalTime > currentTime) {
            printf("%d → %d: Idle.\n", currentTime, p->arrivalTime);
            sleep(p->arrivalTime - currentTime);
            currentTime = p->arrivalTime;
        }

        // record start
        int start = currentTime;

        // resume child; child_main will alarm+pause for burstTime
        kill(p->pid, SIGCONT);

        // wait here until the child's SIGALRM handler exits the child
        waitpid(p->pid, NULL, 0);

        // advance our clock by burstTime
        currentTime += p->burstTime;

        // print after real-time burst completed
        printf("%d → %d: %s Running %s.\n",
               start,
               currentTime,
               p->name,
               p->description);

        // accumulate waiting time = start − arrival
        totalWaitingTime += (start - p->arrivalTime);
    }

    // Footer & summary
    printf("\n──────────────────────────────────────────────\n");
    printf(">> Engine Status  : Completed\n");
    printf(">> Summary        :\n");
    double avg = (double)totalWaitingTime / processCount;
    printf("   └─ Average Waiting Time : %.2f time units\n", avg);
    printf(">> End of Report\n");
    printf("══════════════════════════════════════════════\n");
}

void runCPUScheduler(char* processesCsvFilePath, int timeQuantum) {
    // Parse CSV into processes[]
    parseProcessesFromFile(processesCsvFilePath);

    // For each scheduling algorithm:
    for (int alg = 0; alg < 4; alg++) {
        // Reset per-process fields:
        for (int i = 0; i < processCount; i++) {
            processes[i].remainingTime = processes[i].burstTime;
            processes[i].isFinished = 0;
            processes[i].waitingTime = 0;
            processes[i].started = 0;
        }

        // Fork one child per process and immediately SIGSTOP them:
        for (int i = 0; i < processCount; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                child_main(&processes[i]);
                _exit(0);
            } else {
                processes[i].pid = pid;
                kill(pid, SIGSTOP);
            }
        }

        // c) Call the right scheduler function:
        switch (alg) {
            case 0: run_fcfs(processes, processCount); break;
            case 1: run_sjf(processes, processCount); break;
            case 2: run_priority(processes, processCount); break;
            case 3: run_rr(processes, processCount, timeQuantum); break;
        }

        // Now all children have exited. Loop to the next algorithm.
    }
}