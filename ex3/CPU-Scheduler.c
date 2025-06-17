//cpu
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

typedef struct process {
    pid_t pid;
    char name[MAX_NAME];
    char description[MAX_DESCRIPTION];
    int arrivalTime; 
    int burstTime; 
    int priority; 
    int remainingTime;
} Process;

// function that parse the  file and fill the processes structure array
int parseProcessesFromFile(const char *file_path, Process processes[]) {
    //open the CSV file for reading
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Failed to open CSV file");
        exit(1);
    }
    // read line by line and parse each line into struct
    char line[MAX_LINE];
    int count = 0; // process counter

    // read each line from the file and init the fields
    while (fgets(line, sizeof(line), file) && count < MAX_PROCESSES) {
        char *token = strtok(line, ","); //holds lines in token and cut it by ','
        if (!token) continue;
        strncpy(processes[count].name, token, MAX_NAME);

        token = strtok(NULL, ","); // holds lines in token and cut it by ','
        if (!token) continue;
        strncpy(processes[count].description, token, MAX_DESCRIPTION); 

        token = strtok(NULL, ","); 
        if (!token) continue;
        processes[count].arrivalTime = atoi(token); //convert to an integer

        token = strtok(NULL, ","); 
        if (!token) continue;
        processes[count].burstTime = atoi(token); //convert to an integer
        processes[count].remainingTime = processes[count].burstTime; // initialize the remaining time to the burst time

        token = strtok(NULL, ",");
        if (!token) continue;
        processes[count].priority = atoi(token); //convert to an integer

        count++;
    }

    fclose(file);
    // return the number of the processes
    return count;
}

// this function print the header of the process
void printHeader(const char *scheduler_name) {
    write(1, "══════════════════════════════════════════════\n", strlen("══════════════════════════════════════════════\n"));
    char header[100];
    snprintf(header, sizeof(header), ">> Scheduler Mode : %s\n", scheduler_name);
    write(1, header, strlen(header));
    write(1, ">> Engine Status  : Initialized\n", strlen(">> Engine Status  : Initialized\n"));
    write(1, "──────────────────────────────────────────────\n", strlen("──────────────────────────────────────────────\n"));
    write(1, "\n", 1);
}

// this function print the end of the process
void printSummary(const char *label, float value, int is_integer) {
    write(1, "\n", 1);
    write(1, "──────────────────────────────────────────────\n", strlen("──────────────────────────────────────────────\n"));
    write(1, ">> Engine Status  : Completed\n", strlen(">> Engine Status  : Completed\n"));

    char summary[200];
    if (is_integer) {
        snprintf(summary, sizeof(summary), ">> Summary        :\n   └─ %s : %d time units\n", label, (int)value);
    } else {
        snprintf(summary, sizeof(summary), ">> Summary        :\n   └─ %s : %.2f time units\n", label, value);
    }
    write(1, summary, strlen(summary));
    if (is_integer) {
     write(1, "\n", 1);
    }
    write(1, ">> End of Report\n", strlen(">> End of Report\n"));
    write(1, "══════════════════════════════════════════════\n", strlen("══════════════════════════════════════════════\n"));
    write(1, "\n", 1);
}

// while we have free time in the CPU we call to this function
void idleTime(int start, int end) {
    char idle_msg[MAX_DESCRIPTION];
    snprintf(idle_msg, sizeof(idle_msg), "%d → %d: Idle.\n", start, end);
    write(1, idle_msg, strlen(idle_msg));
}

// Bublle sort to sort the array processes by arrivle time
void BsortByArrivalTime(Process processes[], int processCount) {
    for (int i = 0; i < processCount - 1; i++) {
        for (int j = 0; j < processCount - i - 1; j++) {
            if (processes[j].arrivalTime > processes[j+1].arrivalTime) {
                Process change = processes[j];
                processes[j] = processes[j+1];
                processes[j+1] = change;
            }
        }
    }
}

void signal_handler(int signum) {
    // Empty handler just to catch SIGALRM and continue after pause()
}

// init the signals in process by sleeping for its burst time and hendle the sig_alarm
void initProcessAsChild(Process *process, int *currentTime, int *totalWaitingTime) {
    pid_t pid = fork();
    // if child process
    if (pid == 0) {
        // init fields of sig action 
        struct sigaction sa;
        sa.sa_handler = signal_handler; 
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGALRM, &sa, NULL); 
        alarm(process->burstTime); // we want that the child process will wait the time of its burst
        pause(); // the process will wait until the it get signal after the burst time
        exit(0);
    } else {
        // holds the pid of the process to know when to envoke it
        process->pid = pid; 
        char exec_msg[200];
        snprintf(exec_msg, sizeof(exec_msg), "%d → %d: %s Running %s.\n",
                 *currentTime, *currentTime + process->burstTime,
                 process->name, process->description);
        write(1, exec_msg, strlen(exec_msg));

        // increase the total wait time
        *totalWaitingTime += (*currentTime - process->arrivalTime); 
        // the process sleeps during the running time of the child process
        sleep(process->burstTime); 
        // receive a signal to finish the running time
        kill(pid, SIGALRM); 
        waitpid(pid, NULL, 0);
        // update the current time by the running of the current process
        *currentTime += process->burstTime; 
    }
}

// function to run the algorithem of FCFS
void fcfsAlgorithem(Process processes[], int processCount) {
    // Sort processes by arrival time
    BBsortByArrivalTime(processes, processCount); 
    print_header("FCFS");

    // init fields
    int currentTime = 0;
    int totalWaitingTime = 0;

    for (int i = 0; i < processCount; i++) {
        if (processes[i].arrivalTime > currentTime) {
            idleTime(currentTime, processes[i].arrivalTime); 
            currentTime = processes[i].arrivalTime; // advance the current time to the process's arrival time
        }
        initProcessAsChild(&processes[i], &currentTime, &totalWaitingTime);
    }

    float avg_waiting = (float)totalWaitingTime / processCount;
    printBottom("Average Waiting Time", avg_waiting, 0); // print the summary of the scheduler
}

// function to run the algorithem of SJF
void sjfAlgorithem(Process processes[], int processCount) {
    print_header("SJF"); 

    // initialize values for SJF scheduling
    int currentTime = 0; 
    int totalWaitingTime = 0;
    int completed = 0; 
    int remaining_processes[MAX_PROCESSES] = {0}; // array to keep track of remaining processes
    int max_burstTime = 0;

    // find the maximum burst time among all processes
    for (int i = 0; i < processCount; i++) {
        if (processes[i].burstTime > max_burstTime) {
            max_burstTime = processes[i].burstTime;
        }
    }

    // loop until all processes are completed
    while (completed < processCount) {
        int index_with_min_burstTime = -1; // index of the process with minimum burst time
        int min_burstTime = max_burstTime + 1; // initialize min_burstTime to a value greater than any burst time
        int next_arrivalTime = -1; // track the next arrival time for idle periods

        // find the process with minimum burst time that has arrived
        for (int i = 0; i < processCount; i++) {
            if (!remaining_processes[i] && processes[i].arrivalTime <= currentTime) {
                // if the process is not completed and has arrived, we want to check its burst time
                if (processes[i].burstTime < min_burstTime) {
                    min_burstTime = processes[i].burstTime; // update the minimum burst time
                    index_with_min_burstTime = i; // update the index of the process with minimum burst time
                } else if (processes[i].burstTime == min_burstTime
                           && processes[i].arrivalTime < processes[index_with_min_burstTime].arrivalTime) {
                    // if the burst time is equal, we want to choose the process that arrived first
                    index_with_min_burstTime = i; // update the index of the process with minimum burst time
                }
            } else if (!remaining_processes[i] && (next_arrivalTime == -1 || processes[i].arrivalTime < next_arrivalTime)) {
                // track the next arrival time for idle periods
                next_arrivalTime = processes[i].arrivalTime;
            }
        }

        if (index_with_min_burstTime == -1) {
            // if no process is ready, print idle until the next process arrives
            if (next_arrivalTime > currentTime) {
                idleTime(currentTime, next_arrivalTime);
                currentTime = next_arrivalTime;
            } else {
                currentTime++;
            }
            continue;
        }

        // else we have a process to run, we want to simulate the process execution
        initProcessAsChild(&processes[index_with_min_burstTime], &currentTime, &totalWaitingTime);
        remaining_processes[index_with_min_burstTime] = 1; // mark the process as completed
        completed++; // increment the number of completed processes
    }

    float avg_waiting = (float)totalWaitingTime / processCount;
    printBottom("Average Waiting Time", avg_waiting, 0); // print the summary of the scheduler
}

void priorityAlgorithem(Process processes[], int processCount) {
    print_header("Priority"); // print the header for the Priority scheduler

    // initialize values for Priority scheduling
    int currentTime = 0; // we want to keep track of the current time in the scheduler for future calculations
    int totalWaitingTime = 0; // total waiting time for all processes for calculating the average waiting time
    int completed = 0; // number of processes completed
    int remaining_processes[MAX_PROCESSES] = {0}; // array to keep track of remaining processes
    int max_priority = 0; // we want to keep track of the maximum priority of the processes

    // find the maximum priority among all processes
    for (int i = 0; i < processCount; i++) {
        if (processes[i].priority > max_priority) {
            max_priority = processes[i].priority;
        }
    }

    // loop until all processes are completed
    while (completed < processCount) {
        int index_with_min_priority = -1; // index of the process with minimum priority
        int min_priority = max_priority + 1; // initialize min_priority to a value greater than any priority
        int next_arrivalTime = -1; // track the next arrival time for idle periods

        // find the process with minimum priority that has arrived
        for (int i = 0; i < processCount; i++) {
            if (!remaining_processes[i] && processes[i].arrivalTime <= currentTime) {
                // if the process is not completed and has arrived, we want to check its priority
                if (processes[i].priority < min_priority) {
                    min_priority = processes[i].priority; // update the minimum priority
                    index_with_min_priority = i; // update the index of the process with minimum priority
                } else if (processes[i].priority == min_priority
                           && processes[i].arrivalTime < processes[index_with_min_priority].arrivalTime) {
                    // if the priority is equal, we want to choose the process that arrived first
                    index_with_min_priority = i; // update the index of the process with minimum priority
                }
            } else if (!remaining_processes[i] && (next_arrivalTime == -1 || processes[i].arrivalTime < next_arrivalTime)) {
                // track the next arrival time for idle periods
                next_arrivalTime = processes[i].arrivalTime;
            }
        }

        if (index_with_min_priority == -1) {
            // if no process is ready, print idle until the next process arrives
            if (next_arrivalTime > currentTime) {
                idleTime(currentTime, next_arrivalTime);
                currentTime = next_arrivalTime;
            } else {
                currentTime++;
            }
            continue;
        }

        // else we have a process to run, we want to simulate the process execution
        initProcessAsChild(&processes[index_with_min_priority], &currentTime, &totalWaitingTime);
        remaining_processes[index_with_min_priority] = 1; // mark the process as completed
        completed++; // increment the number of completed processes
    }

    float avg_waiting = (float)totalWaitingTime / processCount;
    printBottom("Average Waiting Time", avg_waiting, 0); // print the summary of the scheduler
}

void initProcessAsChildRR(Process *process, int *currentTime, int *totalWaitingTime, int sleep_time) {
    // simulate a process execution for Round Robin for sleep_time seconds
    pid_t pid = fork();
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = signal_handler; // ignore the alarm signal in the child process
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGALRM, &sa, NULL);
        alarm(sleep_time); // the child process "works" for sleep_time
        pause();
        exit(0);
    } else {
        process->pid = pid;
        char exec_msg[200];
        snprintf(exec_msg, sizeof(exec_msg), "%d → %d: %s Running %s.\n",
                 *currentTime, *currentTime + sleep_time,
                 process->name, process->description);
        write(1, exec_msg, strlen(exec_msg));

        *totalWaitingTime += (*currentTime - process->arrivalTime);

        sleep(sleep_time);
        kill(pid, SIGALRM);
        waitpid(pid, NULL, 0);

        *currentTime += sleep_time;
    }
}

void RRAlgorithem(Process processes[], int processCount, int time_quantum) {
    print_header("Round Robin");

    int currentTime = 0;
    int completed = 0;
    int remainingTime[MAX_PROCESSES];
    int completed_processes[MAX_PROCESSES] = {0};
    int totalWaitingTime = 0;

    for (int i = 0; i < processCount; i++) {
        remainingTime[i] = processes[i].burstTime;
    }

    int queue[MAX_PROCESSES];
    int front = 0, rear = -1, queue_size = 0;
    int in_queue[MAX_PROCESSES] = {0};

    while (completed < processCount) {
        for (int i = 0; i < processCount; i++) {
            if (!completed_processes[i] && processes[i].arrivalTime <= currentTime && 
                !in_queue[i] && remainingTime[i] > 0) {
                rear = (rear + 1) % MAX_PROCESSES;
                queue[rear] = i;
                queue_size++;
                in_queue[i] = 1;
            }
        }

        if (queue_size == 0) {
            int next_arrival = -1;
            for (int i = 0; i < processCount; i++) {
                if (!completed_processes[i] && processes[i].arrivalTime > currentTime) {
                    if (next_arrival == -1 || processes[i].arrivalTime < next_arrival) {
                        next_arrival = processes[i].arrivalTime;
                    }
                }
            }
            if (next_arrival != -1) {
                idleTime(currentTime, next_arrival);
                currentTime = next_arrival;
            }
            continue;
        }

        int process_index = queue[front];
        front = (front + 1) % MAX_PROCESSES;
        queue_size--;
        in_queue[process_index] = 0;

        int exec_time = (remainingTime[process_index] < time_quantum) ? 
                        remainingTime[process_index] : time_quantum;

                         int start_time = currentTime; 
                        int next_time = currentTime + exec_time;
  
                        initProcessAsChildRR(&processes[process_index], &currentTime, &totalWaitingTime, exec_time);

        remainingTime[process_index] -= exec_time;

        if (remainingTime[process_index] == 0) {
            completed_processes[process_index] = 1;
            completed++;
        } else {
        for (int i = 0; i < processCount; i++) {
            if (!completed_processes[i] && processes[i].arrivalTime > start_time &&
                processes[i].arrivalTime < currentTime && !in_queue[i] && remainingTime[i] > 0) {
                rear = (rear + 1) % MAX_PROCESSES;
                queue[rear] = i;
                queue_size++;
                in_queue[i] = 1;
            }
        }
            rear = (rear + 1) % MAX_PROCESSES;
            queue[rear] = process_index;
            queue_size++;
            in_queue[process_index] = 1;
        }
    }

    printBottom("Total Turnaround Time", currentTime, 1);
}

void runCPUScheduler(char* processesCsvFilePath, int timeQuantum) {
    Process processes[MAX_PROCESSES];
    int count = parseProcessesFromFile(processesCsvFilePath, processes);
    fcfsAlgorithem(processes, count);
    sjfAlgorithem(processes, count);
    priorityAlgorithem(processes, count);
    RRAlgorithem(processes, count, timeQuantum);
}