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
    int rrQuantum;  

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


// SIGCONT handler for RR.
// Runs for min(remainingTime, RR_QUANTUM), then returns to pause().
void handle_sigcont_rr(int signum) {
    Process *pr = find_process_by_pid(getpid());
    if (!pr) _exit(1);
    int slice = pr->remainingTime < pr->rrQuantum
                ? pr->remainingTime
                : pr->rrQuantum;
    alarm(slice);
    pause();
}



// SIGALRM handler for RR.
// If more time remains, subtract the slice and self‐stop (preempt).
// Otherwise, exit (job complete).
void handle_sigalrm_rr(int signum) {
    Process *pr = find_process_by_pid(getpid());
    if (!pr) _exit(1);
    if (pr->remainingTime > pr->rrQuantum) {
        pr->remainingTime -= pr->rrQuantum;
        raise(SIGSTOP);
    } else {
        _exit(0);
    }
}

// Child entry point for RR.
// Installs the RR‐specific signal handlers and stops itself until scheduler SIGCONT.
void child_main_rr(Process *process) {
    // initialize remainingTime if not already set
    process->remainingTime = process->burstTime;

    // install SIGCONT handler
    struct sigaction sa_cont = { .sa_handler = handle_sigcont_rr };
    sigemptyset(&sa_cont.sa_mask);
    sigaction(SIGCONT, &sa_cont, NULL);

    // install SIGALRM handler
    struct sigaction sa_alrm = { .sa_handler = handle_sigalrm_rr };
    sigemptyset(&sa_alrm.sa_mask);
    sigaction(SIGALRM, &sa_alrm, NULL);

    // stop immediately until first SIGCONT
    raise(SIGSTOP);

    // never returns: handle_sigcont_rr → alarm → pause → handle_sigalrm_rr
    for (;;)
        pause();
}

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
        printf("%d → %d: %s Running %s.\n", start, currentTime, p->name, p->description);

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

// SJF algorithem
void run_sjf(Process *processes, int processCount) {
    // Header
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : SJF\n");
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");

    // Sort the by arrival time to insert the process by their order
    sortMode = SORT_BY_ARRIVAL;
    qsort(processes, processCount, sizeof(Process), cmp_process);

    int currentTime = 0; // display the current time
    int totalWaitingTime = 0; // counte the wating time of all the processes
    int nextIndex = 0; // manage the 

    // readyQueue holds arrived but not yet run copies
    Process readyQueue[MAX_PROCESSES];
    int readyCount = 0; // pointer for the queue

    // Loop until we've dispatched all
    while (nextIndex < processCount || readyCount > 0) {
        // Enqueue all newly arrived every iteration
        while (nextIndex < processCount &&
               processes[nextIndex].arrivalTime <= currentTime)
        {
            readyQueue[readyCount++] = processes[nextIndex++];
        }

        // f nothing ready so print idle
        if (readyCount == 0) {
            // hold the arrivel time of the next process 
            int nextArrival = processes[nextIndex].arrivalTime;
            printf("%d → %d: Idle.\n", currentTime, nextArrival);
            sleep(nextArrival - currentTime);
            currentTime = nextArrival;
            continue;
        }

        // Pick shortest job sort readyQueue by burstTime
        sortMode = SORT_BY_BURST;
        qsort(readyQueue, readyCount, sizeof(Process), cmp_process);

        // Pop first process
        Process p = readyQueue[0];
        memmove(&readyQueue[0], &readyQueue[1], (--readyCount) * sizeof(Process));

        // record start
        int start = currentTime;
        // resume child; child_main will alarm+pause for burstTime
        kill(p.pid, SIGCONT);
        // wait here until the child's SIGALRM handler exits the child
        waitpid(p.pid, NULL, 0);
        // advance our clock by burstTime
        currentTime += p.burstTime;

        // Print after it actually ran
        printf("%d → %d: %s Running %s.\n", start, currentTime, p.name, p.description);

        // Accumulate waiting time
        totalWaitingTime += (start - p.arrivalTime);
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


// PRIORITY algorithem
void run_priority(Process *processes, int processCount) {
    // Header
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : Priority\n");
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");

    // Sort the by arrival time to insert the process by their order
    sortMode = SORT_BY_ARRIVAL;
    qsort(processes, processCount, sizeof(Process), cmp_process);

    int currentTime = 0; // display the current time
    int totalWaitingTime = 0; // counte the wating time of all the processes
    int nextIndex = 0; // manage the 

    // readyQueue holds arrived but not yet run copies
    Process readyQueue[MAX_PROCESSES];
    int readyCount = 0; // pointer for the queue

    // Loop until we've dispatched all
    while (nextIndex < processCount || readyCount > 0) {
        // Enqueue all newly arrived every iteration
        while (nextIndex < processCount &&
               processes[nextIndex].arrivalTime <= currentTime)
        {
            readyQueue[readyCount++] = processes[nextIndex++];
        }

        // if nothing is ready so print idle
        if (readyCount == 0) {
            // hold the arrivel time of the next process 
            int nextArrival = processes[nextIndex].arrivalTime;
            printf("%d → %d: Idle.\n", currentTime, nextArrival);
            sleep(nextArrival - currentTime);
            currentTime = nextArrival;
            continue;
        }

        // Pick shortest job sort readyQueue by burstTime
        sortMode = SORT_BY_PRIORITY;
        qsort(readyQueue, readyCount, sizeof(Process), cmp_process);

        // Pop first process
        Process p = readyQueue[0];
        memmove(&readyQueue[0], &readyQueue[1], (--readyCount) * sizeof(Process));

        // record start
        int start = currentTime;
        // resume child; child_main will alarm+pause for burstTime
        kill(p.pid, SIGCONT);
        // wait here until the child's SIGALRM handler exits the child
        waitpid(p.pid, NULL, 0);
        // advance our clock by burstTime
        currentTime += p.burstTime;

        // Print after it actually ran
        printf("%d → %d: %s Running %s.\n", start, currentTime, p.name, p.description);

        // Accumulate waiting time
        totalWaitingTime += (start - p.arrivalTime);
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

// RR algorithem
void run_rr(Process *processes, int processCount, int timeQuantum) {
    // Header
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : Round Robin\n");
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");

    // Sort by arrival to enqueue in order
    sortMode = SORT_BY_ARRIVAL;
    qsort(processes, processCount, sizeof(Process), cmp_process);

    int currentTime = 0;
    int nextIndex   = 0;

    Process readyQueue[MAX_PROCESSES];
    int     readyCount = 0;

    // Dispatch loop
    while (nextIndex < processCount || readyCount > 0) {
        // Enqueue all arrivals up to currentTime
        while (nextIndex < processCount &&
               processes[nextIndex].arrivalTime <= currentTime)
        {
            readyQueue[readyCount++] = processes[nextIndex++];
        }

        // If nothing ready → Idle
        if (readyCount == 0) {
            int nextArrival = processes[nextIndex].arrivalTime;
            printf("%d → %d: Idle.\n", currentTime, nextArrival);
            sleep(nextArrival - currentTime);
            currentTime = nextArrival;
            continue;
        }

        // Pop the front of readyQueue (FCFS order for RR)
        Process p = readyQueue[0];
        memmove(&readyQueue[0], &readyQueue[1],
                (--readyCount) * sizeof(Process));

        // Compute this slice length
        int slice = p.remainingTime < timeQuantum ? p.remainingTime : timeQuantum;

        // Run it in real time
        int status, start = currentTime;
        kill(p.pid, SIGCONT);                     // child_main_rr will alarm+pause
        waitpid(p.pid, &status, WUNTRACED);        // catch STOP or EXIT
        currentTime += slice;

        // Print after it really ran
        printf("%d → %d: %s Running %s.\n",
               start, currentTime,
               p.name, p.description);

        // If preempted, update remainingTime & requeue
        if (WIFSTOPPED(status)) {
            p.remainingTime -= slice;
            readyQueue[readyCount++] = p;
        }
        // if it exited, we’re done with this process
    }

    // Footer & summary
    printf("\n──────────────────────────────────────────────\n");
    printf(">> Engine Status  : Completed\n");
    printf(">> Summary        :\n");
    printf("   └─ Total Turnaround Time : %d time units\n", currentTime);
    printf(">> End of Report\n");
    printf("══════════════════════════════════════════════\n");
}



void runCPUScheduler(char* processesCsvFilePath, int timeQuantum) {
    // Parse CSV into processes[]
    parseProcessesFromFile(processesCsvFilePath);

    for (int alg = 0; alg < 4; alg++) {
        // 1) Reset per‐process fields
        for (int i = 0; i < processCount; i++) {
            processes[i].remainingTime = processes[i].burstTime;
            processes[i].isFinished     = 0;
            processes[i].waitingTime    = 0;
            processes[i].started        = 0;
            processes[i].rrQuantum    = timeQuantum;
        }

        // 2) Fork each child and STOP it immediately
        for (int i = 0; i < processCount; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // Child: install the right handlers and stop
                if (alg == 3) {
                    // Round‐Robin
                    child_main_rr(&processes[i]);   // uses handle_sigcont_rr / handle_sigalrm_rr
                } else {
                    // FCFS, SJF or Priority
                    child_main(&processes[i]);      // uses handle_sigcont / handle_sigalrm
                }
                _exit(0);
            } 
            // Parent
            processes[i].pid = pid;
            kill(pid, SIGSTOP);
        }

        // 3) Dispatch according to the current algorithm
        switch (alg) {
            case 0: run_fcfs(processes, processCount);      break;
            case 1: run_sjf(processes, processCount);       break;
            case 2: run_priority(processes, processCount);  break;
            case 3: run_rr(processes, processCount, timeQuantum); break;
        }
    }
}
