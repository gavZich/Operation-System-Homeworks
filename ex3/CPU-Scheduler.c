#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_PROCESSES 1000
#define MAX_LINE 256
#define MAX_NAME 50
#define MAX_DESCRIPTION 100

// Proccess struct, contain all the fields for the algorithems
typedef struct {
    pid_t pid;
    char name[MAX_NAME];
    char description[MAX_DESCRIPTION];
    int arrivalTime;
    int burstTime;
    int priority;
    int remainingTime;
} Process;


// Parse and init the data from the file into the structs
int loadProcessesFromCSV(const char *filePath, Process procList[]) {
    FILE *fp = fopen(filePath, "r");
    if (!fp) {
        perror("Unable to open CSV");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < MAX_PROCESSES) {
        char *token = strtok(line, ",");
        if (!token) continue;
        strncpy(procList[count].name, token, MAX_NAME);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(procList[count].description, token, MAX_DESCRIPTION);

        token = strtok(NULL, ",");
        if (!token) continue;
        procList[count].arrivalTime = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        procList[count].burstTime = atoi(token);
        procList[count].remainingTime = procList[count].burstTime;

        token = strtok(NULL, ",");
        if (!token) continue;
        procList[count].priority = atoi(token);

        count++;
    }

    fclose(fp);
    return count;
}

// Printer functions
void printReportHeader(const char *mode) {
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : %s\n", mode);
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");
}

// Printer functions
void printReportFooter(const char *label, float value, int isInt) {
    printf("\n");
    printf("──────────────────────────────────────────────\n");
    printf(">> Engine Status  : Completed\n");
    printf(">> Summary        :\n");
    if (isInt)
        printf("   └─ %s : %d time units\n", label, (int)value);
    else
        printf("   └─ %s : %.2f time units\n", label, value);
    printf(">> End of Report\n");
    printf("══════════════════════════════════════════════\n\n");
}

// handle IDLE time
void simulateIdleTime(int start, int end) {
    printf("%d → %d: Idle.\n", start, end);
}

// simple implement of bubblesort
void sortByArrival(Process p[], int n) {
    for (int i = 0; i < n-1; ++i)
        for (int j = 0; j < n-i-1; ++j)
            if (p[j].arrivalTime > p[j+1].arrivalTime) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }
}

void alarmHandler(int sig) {
    // handler for SIGALRM
}


// execute process and invoke it
void spawnChildProcess(Process *p, int *now, int *waitSum) {
    pid_t pid = fork();
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = alarmHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGALRM, &sa, NULL);
        alarm(p->burstTime);
        pause();
        exit(EXIT_SUCCESS);
    } else {
        p->pid = pid;
        printf("%d → %d: %s Running %s.\n", *now, *now + p->burstTime, p->name, p->description);
        *waitSum += (*now - p->arrivalTime);
        sleep(p->burstTime);
        kill(pid, SIGALRM);
        waitpid(pid, NULL, 0);
        *now += p->burstTime;
    }
}

// manege the same goals but for RR algorithem
void spawnChildProcessRR(Process *p, int *now, int *waitSum, int duration) {
    pid_t pid = fork();
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = alarmHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGALRM, &sa, NULL);
        alarm(duration);
        pause();
        exit(EXIT_SUCCESS);
    } else {
        p->pid = pid;
        char exec[256];
        snprintf(exec, sizeof(exec), "%d → %d: %s Running %s.\n",
                 *now, *now + duration, p->name, p->description);
        write(STDOUT_FILENO, exec, strlen(exec));
        *waitSum += (*now - p->arrivalTime);
        sleep(duration);
        kill(pid, SIGALRM);
        waitpid(pid, NULL, 0);
        *now += duration;
    }
}


// FCFS Algorithem
void runFCFS(Process p[], int count) {
    sortByArrival(p, count);
    printReportHeader("FCFS");

    int time = 0, waitSum = 0;
    for (int i = 0; i < count; ++i) {
        if (p[i].arrivalTime > time) {
            simulateIdleTime(time, p[i].arrivalTime);
            time = p[i].arrivalTime;
        }
        spawnChildProcess(&p[i], &time, &waitSum);
    }

    printReportFooter("Average Waiting Time", (float)waitSum / count, 0);
}

// SJF Algorithem
void runSJF(Process p[], int count) {
    printReportHeader("SJF");

    int time = 0, completed = 0, waitSum = 0;
    int done[MAX_PROCESSES] = {0};

    while (completed < count) {
        int minBurst = __INT_MAX__, index = -1;
        int nextArrival = -1;

        for (int i = 0; i < count; ++i) {
            if (!done[i] && p[i].arrivalTime <= time) {
                if (p[i].burstTime < minBurst || 
                   (p[i].burstTime == minBurst && p[i].arrivalTime < p[index].arrivalTime)) {
                    minBurst = p[i].burstTime;
                    index = i;
                }
            } else if (!done[i] && (nextArrival == -1 || p[i].arrivalTime < nextArrival)) {
                nextArrival = p[i].arrivalTime;
            }
        }

        if (index == -1) {
            simulateIdleTime(time, nextArrival);
            time = nextArrival;
            continue;
        }

        spawnChildProcess(&p[index], &time, &waitSum);
        done[index] = 1;
        completed++;
    }

    printReportFooter("Average Waiting Time", (float)waitSum / count, 0);
}

// Priority Algorithem
void runPriority(Process p[], int count) {
    printReportHeader("Priority");

    int time = 0, completed = 0, waitSum = 0;
    int done[MAX_PROCESSES] = {0};

    while (completed < count) {
        int minPrio = __INT_MAX__, index = -1;
        int nextArrival = -1;

        for (int i = 0; i < count; ++i) {
            if (!done[i] && p[i].arrivalTime <= time) {
                if (p[i].priority < minPrio || 
                   (p[i].priority == minPrio && p[i].arrivalTime < p[index].arrivalTime)) {
                    minPrio = p[i].priority;
                    index = i;
                }
            } else if (!done[i] && (nextArrival == -1 || p[i].arrivalTime < nextArrival)) {
                nextArrival = p[i].arrivalTime;
            }
        }

        if (index == -1) {
            simulateIdleTime(time, nextArrival);
            time = nextArrival;
            continue;
        }

        spawnChildProcess(&p[index], &time, &waitSum);
        done[index] = 1;
        completed++;
    }

    printReportFooter("Average Waiting Time", (float)waitSum / count, 0);
}

// RR Algorithem
void runRR(Process p[], int count, int quantum) {
    printReportHeader("Round Robin");

    int time = 0, completed = 0, waitSum = 0;
    int remaining[MAX_PROCESSES], inQueue[MAX_PROCESSES] = {0}, done[MAX_PROCESSES] = {0};
    int queue[MAX_PROCESSES], front = 0, rear = -1, qSize = 0;

    for (int i = 0; i < count; ++i)
        remaining[i] = p[i].burstTime;

    while (completed < count) {
        for (int i = 0; i < count; ++i) {
            if (!done[i] && !inQueue[i] && p[i].arrivalTime <= time && remaining[i] > 0) {
                rear = (rear + 1) % MAX_PROCESSES;
                queue[rear] = i;
                qSize++;
                inQueue[i] = 1;
            }
        }

        if (qSize == 0) {
            int nextArrival = -1;
            for (int i = 0; i < count; ++i)
                if (!done[i] && p[i].arrivalTime > time && 
                    (nextArrival == -1 || p[i].arrivalTime < nextArrival))
                    nextArrival = p[i].arrivalTime;

            simulateIdleTime(time, nextArrival);
            time = nextArrival;
            continue;
        }

        int idx = queue[front];
        front = (front + 1) % MAX_PROCESSES;
        qSize--;
        inQueue[idx] = 0;

        int execTime = (remaining[idx] < quantum) ? remaining[idx] : quantum;
        spawnChildProcessRR(&p[idx], &time, &waitSum, execTime);
        remaining[idx] -= execTime;

        if (remaining[idx] == 0) {
            done[idx] = 1;
            completed++;
        } else {
            rear = (rear + 1) % MAX_PROCESSES;
            queue[rear] = idx;
            qSize++;
            inQueue[idx] = 1;
        }
    }

    printReportFooter("Total Turnaround Time", time, 1);
}


void runCPUScheduler(char *filePath, int quantum) {
    Process procList[MAX_PROCESSES];
    int count = loadProcessesFromCSV(filePath, procList);

    runFCFS(procList, count);
    runSJF(procList, count);
    runPriority(procList, count);
    runRR(procList, count, quantum);
}
